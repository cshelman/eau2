#pragma once

#include "key.h"
#include <string>
#include <queue>
#include <vector>
#include <mutex>

using namespace std;

enum class MsgType {
    Act, Get, Put, Kill
};

class Message {
public:
    MsgType type;
    Key* key;
    char* contents;

    Message(MsgType t, Key* k) {
        type = t;
        key = k->copy();
        contents = (char*)"";
    }

    Message(MsgType t, Key* k, char* c) {
        type = t;
        key = k->copy();
        contents = new char[strlen(c) + 1];
        strcpy(contents, c);
    }

    ~Message() {
        delete contents;
        delete key;
    }

    Message* copy() {
        Message* ret = new Message(type, key, contents);
        return ret;
    }
};