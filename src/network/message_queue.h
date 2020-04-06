#pragma once

#include "message.h"
#include <string>
#include <queue>
#include <mutex>

using namespace std;

// FIFO queue of messages with push and pop
// idea from Jan Vitek's lecture
class MessageQueue {
public:
    mutex mtx;
    queue<Message*> mq;

    MessageQueue() {}

    void push(Message* msg) {
        mtx.lock();
        mq.push(msg);
        mtx.unlock();
    }

    // returns nullptr when the queue is empty;
    Message* pop() {
        if (mq.size() > 0) {
            mtx.lock();
            Message* ret = mq.front();
            mq.pop();
            mtx.unlock();
            return ret;
        }
        else {
            return nullptr;
        }
    }
};