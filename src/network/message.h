#pragma once

#include "key.h"
#include <string>
#include <queue>
#include <vector>
#include <mutex>

using namespace std;

enum class MsgType {
    Act, Get, Put, Kill, SetRower
};

class Message {
public:
    MsgType type;
    Key* key;
    char* contents;
    size_t sender;

    Message(MsgType t, Key* k, int from) {
        type = t;
        key = k->copy();
        contents = (char*)"";
        sender = from;
    }

    Message(MsgType t, Key* k, char* c, int from) {
        type = t;
        key = k->copy();
        contents = new char[strlen(c) + 1];
        strcpy(contents, c);
        sender = from;
    }

    ~Message() {
        delete contents;
        delete key;
    }

    Message* copy() {
        Message* ret = new Message(type, key, contents, sender);
        return ret;
    }
};