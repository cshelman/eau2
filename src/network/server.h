#pragma once

#include "../dataframe/dataframe.h"
#include "../dataframe/column.h"
#include "node.h"
#include "key.h"
#include "network_server.h"
#include "message.h"
#include "../serializer/serial.h"
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
        Message* kill_msg = new Message(MsgType::Kill, new Key(name), (char*)"KILL");
        for (int i = 0; i < net->num_nodes; i++) {
            net->send_msg(i, kill_msg);
        }
        net->shutdown();
    }

    void run_rower(Key* key) {
        Message* msg = new Message(MsgType::Act, key, (char*)"ACT");
        for (size_t i = 0; i < net->num_nodes; i++) {
            net->send_msg(i, msg);
        }
    }

    DataFrame* get(Key* key) {
        // printf("\n\n\nSTARTING GET in server\n\n\n");
        Message* msg = new Message(MsgType::Get, key, (char*)"GET");

        vector<Column*>* df_cols = new vector<Column*>();
        for (size_t i = 0; i < net->num_nodes; i++) {
            mtx.lock();
            // printf("\nsending get for %s\n", (char*)key->name.c_str());
            net->send_msg(i, msg);
            mtx.unlock();
            while (1) {
                Message* ret_msg = net->recv_master();
                if (ret_msg == nullptr) {
                    delete ret_msg;
                    continue;
                }
                else {
                    vector<Column*>* node_cols = deserialize_col_vector(ret_msg->contents);
                    for (int j = 0; j < node_cols->size(); j++) {
                        df_cols->push_back(node_cols->at(j));
                    }
                    delete ret_msg;
                    break;
                }
            }
        }

        Schema* schema = new Schema();
        schema->num_rows_ = df_cols->at(0)->size();
        DataFrame* df = new DataFrame(*schema);
        for (int i = 0; i < df_cols->size(); i++) {
            df->add_column(df_cols->at(i));
        }
        // printf("\n\nDONE WITH GET in server\n\n\n");
        return df;
    }

    void put(Key* key, DataFrame* df) {
        vector<Column*>* col_arr = new vector<Column*>(*df->col_arr);
        int min_cols_per_node = col_arr->size() / net->num_nodes;
        int rem = col_arr->size() % net->num_nodes;
        printf("min cols: %d\n", min_cols_per_node);

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
            vector<Message*>* messages = parse_msg(MsgType::Put, key, (char*)val.c_str());
            for (int j = 0; j < messages->size(); j++) {
                // printf("sending PUT msg to: %d\n", i);
                net->send_msg(i, messages->at(j));
                // printf("\nsending put: %s\n", messages->at(j)->contents);
            }
            
            delete messages;
        }
        delete col_arr;
    }

    vector<Message*>* parse_msg(MsgType type, Key* key, char* s) {
        vector<Message*>* messages = new vector<Message*>();
        string* str = new string(s);
        int chunk_size = 500;
        if (str->size() < chunk_size) {
            chunk_size = str->size() - 1;
        }
        int start = 0;
        int end = chunk_size;

        while (true) {
            Message* msg = new Message(type, key, (char*)str->substr(start, end - start).c_str());
            messages->push_back(msg);

            if (end == str->size() - 1) {
                break;
            }
            start = end;
            end += chunk_size;
            if (end > str->size()) {
                end = str->size() - 1;
            }
        }
        Message* end_msg = new Message(type, key, (char*)"END");
        messages->push_back(end_msg);
        return messages;
    }
    
    DataFrame* wait_and_get(Key key) {}
};