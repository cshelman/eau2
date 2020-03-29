#pragma once

#include "key_value_store.h"
#include "key.h"
#include <string>
#include <queue>
#include <vector>
#include <mutex>

using namespace std;

enum class MsgType {
    Get, Put, Kill
};

class Message {
public:
    MsgType type;
    Key* key;
    char* contents;

    Message(MsgType t, Key* k) {
        type = t;
        key = k->copy();
    }

    Message(MsgType t, Key* k, char* c) {
        type = t;
        key = k->copy();
        contents = c;
    }

    Message* copy() {
        Message* ret = new Message(type, key, contents);
        return ret;
    }
};

// FIFO queue of messages with push and pop
// idea from Jan Vitek's lecture
class MessageQueue {
public:
    mutex mtx;
    queue<Message*> mq;

    MessageQueue() {}

    void push(Message* msg) {
        mtx.lock();
        mq.push(msg->copy());
        mtx.unlock();
    }

    Message* pop() {
        mtx.lock();
        Message* ret = mq.front();
        mq.pop();
        mtx.unlock();
        return ret;
    }
};

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

    void send_msg(size_t dest, Message* msg) {
        qs->at(dest)->push(msg);
    }

    Message* recv_msg(size_t node) {
        qs->at(node)->pop();
    }

    void send_master(Message* msg) {
        master_queue->push(msg);
    }

    Message* recv_master() {
        master_queue->pop();
    }
};