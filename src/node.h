#pragma once

#include <unordered_map>
#include "key_value_store.h"

using namespace std;

class Node {
public:
    unordered_map<Key, char*> pairs;

    Node() {

    }

    ~Node() {
        delete pairs;
    }

    char* get(Key key) {
        return pairs.at(key);
    }

    void put(Key key, char* data) {
        
    }
};