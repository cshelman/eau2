#pragma once

#include "server.h"
#include "key.h"
#include "message.h"
#include "message_queue.h"
#include <string>
#include <queue>
#include <vector>
#include <mutex>

using namespace std;

class FakeNetwork {
public:
    MessageQueue* master_queue;
    vector<MessageQueue*>* qs;
    size_t num_nodes;

    FakeNetwork(size_t nodes) {
        num_nodes = nodes;
        qs = new vector<MessageQueue*>();
        for (int i = 0; i < num_nodes; i++) {
            qs->push_back(new MessageQueue());
        }
        master_queue = new MessageQueue();
    }

    ~FakeNetwork() {
        delete qs;
    }

    vector<Message*>* parse_msg(MsgType type, Key* key, char* s, size_t sender) {
        vector<Message*>* messages = new vector<Message*>();
        string* str = new string(s);
        int chunk_size = 5000;
        int start = 0;
        int end = chunk_size;

        while (true) {
            Message* msg = new Message(type, key, (char*)str->substr(start, end - start).c_str(), sender);
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
        Message* end_msg = new Message(type, key, (char*)"END", sender);
        messages->push_back(end_msg);
        return messages;
    }

    void send_msg(size_t dest, Message* msg) {
        Message* temp = msg->copy();
        qs->at(dest)->push(temp);
    }

    Message* recv_msg(size_t node) {
        return qs->at(node)->pop();
    }

    void send_master(Message* msg) {
        Message* temp = msg->copy();
        master_queue->push(temp);
    }

    Message* recv_master() {
        Message* temp = master_queue->pop();
        if (temp != nullptr) {
        }
        return temp;
    }
};