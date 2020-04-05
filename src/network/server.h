#pragma once

#include "../dataframe/dataframe.h"
#include "../dataframe/column.h"
#include "node.h"
#include "key.h"
#include "fake_network.h"
#include "message.h"
#include "../serializer/buffer.h"
#include "../serializer/serial.h"
#include <string>
#include <mutex>

using namespace std;

class Server {
public:
    FakeNetwork* net;
    mutex mtx;

    Server(FakeNetwork* network) {
        net = network;
    }

    void shutdown() {
        string name = "";
        Message* kill_msg = new Message(MsgType::Kill, new Key(name));
        for (int i = 0; i < net->num_nodes; i++) {
            net->send_msg(i, kill_msg);
        }
    }

    DataFrame* get(Key* key) {
        Message* msg = new Message(MsgType::Get, key);

        vector<Column*>* df_cols = new vector<Column*>();
        for (size_t i = 0; i < net->num_nodes; i++) {
            mtx.lock();
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
            string val = serialize_col_vector(cols_to_send);
            delete cols_to_send;

            // send serialized package
            vector<Message*>* messages = net->parse_msg(MsgType::Put, key, (char*)val.c_str());
            for (int j = 0; j < messages->size(); j++) {
                net->send_msg(i, messages->at(j));
            }
            
            delete messages;
        }
        delete col_arr;
    }
    
    DataFrame* wait_and_get(Key key) {}
};