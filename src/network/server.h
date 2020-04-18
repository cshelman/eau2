#pragma once

#include "../dataframe/dataframe.h"
#include "../dataframe/column.h"
#include "node.h"
#include "key.h"
#include "network_server.h"
#include "message.h"
#include "../serializer/serial.h"
#include "../rowers/word_count.h"
#include "../rowers/find_projects.h"
#include <string>
#include <mutex>
#include <vector>

using namespace std;

// The master server used to orchestrate the clients for eau2
class Server {
public:
    NetworkServer* net;
    mutex mtx;

    Server(NetworkServer* network) {
        net = network;
        net->startup();
        net->registration();
    }

    // Shuts down (or instructs to shut down) the following:
    // - Clients
    // - Nodes
    // - Network Server
    void shutdown() {
        string name = "KILL";
        Key* key = new Key(name);
        Message* kill_msg = new Message(MsgType::Kill, key, (char*)"KILL", -1);
        for (int i = 0; i < net->num_nodes; i++) {
            net->send_msg(i, kill_msg);
        }
        delete key;
        delete kill_msg;
        net->shutdown();
    }

    // Instructs each client to run their rower against the given dataframe
    // Receives resulting rowers after they are run against the df
    // Joins the rowers and returns the final result
    void run_rower(Key* key, Rower* rower) {

        // Send the instruction to run the rower
        Message* msg = new Message(MsgType::Act, key, (char*)"ACT", -1);
        for (size_t i = 0; i < net->num_nodes; i++) {
            net->send_msg(i, msg);
        }
        delete msg;

        // Receive the rowers in pieces then put the pieces together
        vector<Rower*>* rowers = new vector<Rower*>();
        map<size_t, string>* response_map = new map<size_t, string>();
        size_t end_count = 0;
        while (true) {
            Message* ret_msg = net->recv_master();
            if (ret_msg == nullptr) {
                delete ret_msg;
                continue;
            }
            else if (strcmp(ret_msg->contents, "END") == 0) {
                Rower* r = rower->deserialize((char*)response_map->at(ret_msg->sender).c_str());

                rowers->push_back(r);
                delete ret_msg;
                end_count++;
                if (end_count >= net->num_nodes) {
                    break;
                }
            }
            else {
                if (response_map->count(ret_msg->sender) > 0) {
                    string s(ret_msg->contents);
                    response_map->at(ret_msg->sender) = response_map->at(ret_msg->sender).append(s);
                }
                else {
                    string s(ret_msg->contents);
                    response_map->insert({ret_msg->sender, s});
                }
                
                delete ret_msg;
                continue;
            }
        }

        // Join the rowers
        for (int i = 0; i < rowers->size(); i++) {
            rower->join_delete(rowers->at(i));
        }
        rowers->clear();
        delete rowers;
        delete response_map;
    }

    // Retrieves a previously inserted dataframe from the distributed KV store
    DataFrame* get(Key* key) {
        Message* msg = new Message(MsgType::Get, key, (char*)"GET", -1);

        vector<Column*>* df_cols = new vector<Column*>();
        map<size_t, string>* response_map = new map<size_t, string>();
        for (size_t i = 0; i < net->num_nodes; i++) {
            mtx.lock();
            net->send_msg(i, msg);
            mtx.unlock();
            while (true) {
                Message* ret_msg = net->recv_master();
                if (ret_msg == nullptr) {
                    delete ret_msg;
                    continue;
                }
                else if (strcmp(ret_msg->contents, "END") == 0) {
                    vector<Column*>* node_cols = deserialize_col_vector((char*)response_map->at(ret_msg->sender).c_str());

                    // if these are the first columns that come back
                    if (df_cols->size() == 0) {
                        for (int j = 0; j < node_cols->size(); j++) {
                            df_cols->push_back(node_cols->at(j));
                        }
                    }
                    // for all nodes but the first ones
                    else {
                        for (int j = 0; j < node_cols->size(); j++) {
                            df_cols->at(j)->append(node_cols->at(j));
                        }
                    }
                    delete ret_msg;
                    break;
                }
                else {
                    if (response_map->count(ret_msg->sender) > 0) {
                        string s(ret_msg->contents);
                        response_map->at(ret_msg->sender) = response_map->at(ret_msg->sender).append(s);
                    }
                    else {
                        string s(ret_msg->contents);
                        response_map->insert({ret_msg->sender, s});
                    }
                    delete ret_msg;
                    continue;
                }
            }
        }

        // Reconstruct the dataframe from the received subcolumns
        Schema* schema = new Schema();
        schema->num_rows_ = df_cols->at(0)->size();
        DataFrame* df = new DataFrame(*schema);
        for (int i = 0; i < df_cols->size(); i++) {
            df->add_column(df_cols->at(i));
        }
        return df;
    }

    // Puts a given dataframe into the distributed KV store
    // Splitting the data up by creating subcolumns
    void put(Key* key, DataFrame* df) {
        vector<Column*>* col_arr = new vector<Column*>(*df->col_arr);
        int entries_per_node = df->nrows() / net->num_nodes;
        int rem = df->nrows() % net->num_nodes;
        
        // to each client in the network:
        for (int i = 0; i < net->num_nodes; i++) {
            vector<Column*>* sub_cols = new vector<Column*>();

            // get the subsection of the columns to send
            int offset = i * entries_per_node;
            if (i == net->num_nodes - 1) {
                // this changes the meaning of this variable
                entries_per_node += rem;
            }
            for (int j = 0; j < col_arr->size(); j++) {
                //gets rows from (i * entries_per_node) to (i * entries_per_node) + entries_per_node
                Column* subset = col_arr->at(j)->get_subset(offset, offset +  entries_per_node);
                sub_cols->push_back(subset);
            }

            // serialize sub columns
            string val = "";
            if (sub_cols->size() > 0) {
                val = serialize_col_vector(sub_cols);
            }
            for (int j = 0; j < sub_cols->size(); j++) {
                if (sub_cols->at(j)->get_type() == 'I') {
                    delete sub_cols->at(j)->as_int();
                }
                else if (sub_cols->at(j)->get_type() == 'F') {
                    delete sub_cols->at(j)->as_float();
                }
                else if (sub_cols->at(j)->get_type() == 'B') {
                    delete sub_cols->at(j)->as_bool();
                }
                else if (sub_cols->at(j)->get_type() == 'S') {
                    delete sub_cols->at(j)->as_string();
                }
                else {
                    printf("Invalid column type when deleting sub_cols after put\n");
                }
            }
            sub_cols->clear();
            delete sub_cols;

            // send serialized package
            vector<Message*>* messages = parse_msg(MsgType::Put, key, (char*)val.c_str(), -1);
            for (int j = 0; j < messages->size(); j++) {
                net->send_msg(i, messages->at(j));
                delete messages->at(j);
            }
            
            delete messages;
        }
        
        delete col_arr;
    }

    // Sends the given rower to each client
    // The clients then hold the rower in memory until triggered to be sent
    void set_rower(Rower* r) {
        Key* key = new Key("SET_ROWER");
        string* serialized_rower = r->serialize();
        vector<Message*>* messages = parse_msg(MsgType::SetRower, key, (char*)serialized_rower->c_str(), -1);
        for (int i = 0; i < net->num_nodes; i++) {
            // send serialized package
            for (int j = 0; j < messages->size(); j++) {
                net->send_msg(i, messages->at(j));
            }
        }
        delete serialized_rower;
        for (int i = 0; i < messages->size(); i++) {
            delete messages->at(i);
        }
        delete messages;
        delete key;
    }

    // Splits up really large messages into smaller messages so they can be
    // Sent over the network, TODO: Move to util file
    vector<Message*>* parse_msg(MsgType type, Key* key, char* s, size_t sender) {
        vector<Message*>* messages = new vector<Message*>();
        string* str = new string(s);
        int chunk_size = 500;
        if (str->size() < chunk_size) {
            chunk_size = str->size() - 1;
        }
        int start = 0;
        int end = chunk_size;

        while (true) {
            Message* msg = new Message(type, key, (char*)str->substr(start, end - start).c_str(), sender);
            messages->push_back(msg);

            if (end == str->size()) {
                break;
            }
            start = end;
            end += chunk_size;
            if (end > str->size()) {
                end = str->size();
            }
        }
        Message* end_msg = new Message(type, key, (char*)"END", sender);
        messages->push_back(end_msg);
        delete str;
        return messages;
    }
};