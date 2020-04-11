#pragma once

#include "../dataframe/dataframe.h"
#include "../dataframe/column.h"
#include "node.h"
#include "key.h"
#include "network_server.h"
#include "message.h"
#include "../serializer/serial.h"
#include "../rowers/word_count.h"
#include <string>
#include <mutex>
#include <vector>

using namespace std;

class Server {
public:
    NetworkServer* net;
    mutex mtx;

    Server(NetworkServer* network) {
        net = network;
        net->startup();
        net->registration();
    }

    void shutdown() {
        string name = "KILL";
        Message* kill_msg = new Message(MsgType::Kill, new Key(name), (char*)"KILL", -1);
        for (int i = 0; i < net->num_nodes; i++) {
            net->send_msg(i, kill_msg);
        }
        net->shutdown();
    }

    Rower* run_rower(Key* key, Rower* rower) {
        Message* msg = new Message(MsgType::Act, key, (char*)"ACT", -1);
        for (size_t i = 0; i < net->num_nodes; i++) {
            net->send_msg(i, msg);
        }

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
                if (end_count >= 3) {
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

        for (int i = 0; i < rowers->size(); i++) {
            WordCountRower* wr = dynamic_cast<WordCountRower*>(rower);
            rower->join_delete(rowers->at(i));
        }
        
        return rower;
    }

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
                    for (int j = 0; j < node_cols->size(); j++) {
                        df_cols->push_back(node_cols->at(j));
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

        Schema* schema = new Schema();
        schema->num_rows_ = df_cols->at(0)->size();
        DataFrame* df = new DataFrame(*schema);
        for (int i = 0; i < df_cols->size(); i++) {
            df->add_column(df_cols->at(i));
        }
        return df;
    }

    void put(Key* key, DataFrame* df) {
        vector<Column*>* col_arr = new vector<Column*>(*df->col_arr);
        int min_cols_per_node = col_arr->size() / net->num_nodes;
        int rem = col_arr->size() % net->num_nodes;
        for (int i = 0; i < net->num_nodes; i++) {
            // calculate number of columns to send to this node
            int cur_size = min_cols_per_node;
            if (rem > 0) {
                cur_size++;
                rem--;
            }

            // add the columns to the column vector to send
            vector<Column*>* cols_to_send = new vector<Column*>();
            for (int j = 0; j < cur_size; j++) {
                cols_to_send->push_back(col_arr->front());
                col_arr->erase(col_arr->begin());
            }

            // serialize columns
            string val = "";
            if (cols_to_send->size() > 0) {
                val = serialize_col_vector(cols_to_send);
            }
            delete cols_to_send;

            // send serialized package
            vector<Message*>* messages = parse_msg(MsgType::Put, key, (char*)val.c_str(), -1);
            for (int j = 0; j < messages->size(); j++) {
                net->send_msg(i, messages->at(j));
            }
            
            delete messages;
        }
        delete col_arr;
    }

    void set_rower(Rower* r) {
        Key* key = new Key("SET_ROWER");
        vector<Message*>* messages = parse_msg(MsgType::SetRower, key, r->serialize(), -1);
        for (int i = 0; i < net->num_nodes; i++) {
            // send serialized package
            for (int j = 0; j < messages->size(); j++) {
                net->send_msg(i, messages->at(j));
            }
        }
        delete messages;
    }

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
        return messages;
    }
    
    DataFrame* wait_and_get(Key key) {}
};