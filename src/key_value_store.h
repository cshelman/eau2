#pragma once

#include "dataframe.h"
#include "node.h"
#include "key.h"
#include "fake_network.h"
#include "serializer/buffer.h"
#include "serializer/serial.h"
#include <string>
#include <mutex>

using namespace std;

class KVStore {
public:
    FakeNetwork* net;
    size_t chunk_threshold;
    int next_full_node;
    mutex mtx;

    KVStore(FakeNetwork* network) {
        net = network;
        chunk_threshold = net->num_nodes * 100;
        next_full_node = 0;
    }

    void shutdown() {
        Message* kill_msg = new Message(MsgType::Kill, nullptr);
        for (int i = 0; i < net->num_nodes; i++) {
            net->send_msg(i, kill_msg);
        }
    }

    DataFrame* get(Key* key) {
        Message* msg = new Message(MsgType::Get, key);
        Buffer* buf = new Buffer();
        // pull the byte sequence chunks from each node sequentially
        // add the result to the buffer each time
        for (size_t i = 0; i < net->num_nodes; i++) {
            mtx.lock();
            net->send_msg(i, msg);
            Message* ret_msg = net->recv_master();
            buf->add(ret_msg->contents);
            mtx.unlock();
        }
        // deserialize the df from bytes and return
        return deserialize_dataframe(buf->val);
    }

    void put(Key* key, DataFrame* df) {
        Buffer* buf;
        // serialize the df into bytes
        serialize_dataframe(df, buf);
        // if serialized length is larger than the chunking threshold
        if (buf->size > chunk_threshold) {
            // split up the byte array into (number of nodes) chunks
            // put into each array according to the node index, using the same key
            size_t chunk_size = (buf->size / net->num_nodes) + 1;
            char* temp = new char[chunk_size];
            for (size_t i = 0; i < net->num_nodes; i++) {
                memset(temp, '\0', chunk_size + 1);
                size_t offset = i * chunk_size;
                if (i == net->num_nodes - 1) {
                    // on the last copy only go to the end of string, not full chunk size
                    memcpy(temp, buf->val + offset, buf->size - offset);
                }
                else {
                    memcpy(temp, buf->val + offset, chunk_size);
                }

                Message* msg = new Message(MsgType::Put, key, temp);
                net->send_msg(i, msg);
            }
        }
        else {
            // only send the bytes to one of the nodes
            Message* msg = new Message(MsgType::Put, key, buf->val);
            net->send_msg(next_full_node, msg);
            next_full_node = (next_full_node + 1) % net->num_nodes;
        }
    }
    
    DataFrame* wait_and_get(Key key) {}
};