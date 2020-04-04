#pragma once

#include "../dataframe/dataframe.h"
#include "../dataframe/column.h"
#include "node.h"
#include "key.h"
#include "fake_network.h"
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
        // printf("\tMaster attempting to shutdown network (%ld)...\n", net->num_nodes);
        string name = "";
        Message* kill_msg = new Message(MsgType::Kill, new Key(name));
        for (int i = 0; i < net->num_nodes; i++) {
            // printf("\t\tSent kill msg to %d...\n", i);
            net->send_msg(i, kill_msg);
        }
    }

    DataFrame* get(Key* key) {
        Message* msg = new Message(MsgType::Get, key);
        // printf("Starting to pull from nodes...\n");

        vector<Column*>* df_cols = new vector<Column*>();
        for (size_t i = 0; i < net->num_nodes; i++) {
            // printf("About to poll node %ld\n", i);
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
                    // printf("Server received from node %ld: `%s`\n", i, ret_msg->contents);
                    // printf("server recieved `%s` from %ld\n", ret_msg->contents, i);
                    vector<Column*>* node_cols = deserialize_col_vector(ret_msg->contents);
                    // df_cols->insert(df_cols->end(), node_cols->begin(), node_cols->end());
                    for (int j = 0; j < node_cols->size(); j++) {
                        // printf("%d: ADDING COLUMN\n", i);
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

            // printf("Server putting %ld cols\n", cols_to_send->size());

            // serialize columns
            string val = serialize_col_vector(cols_to_send);
            delete cols_to_send;

            // send serialized package
            Message* msg = new Message(MsgType::Put, key, (char*)val.c_str());
            net->send_msg(i, msg);
            delete msg;
        }
        delete col_arr;
    }
    
    DataFrame* wait_and_get(Key key) {}
};