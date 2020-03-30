#pragma once

#include "key_value_store.h"
#include <unordered_map>
#include <stdio.h>

using namespace std;

class Node {
public:
    unordered_map<Key*, char*>* pairs;
    size_t id;

    Node(size_t node_id) {
        id = node_id;
        pairs = new unordered_map<Key*, char*>();
    }

    ~Node() {
        delete pairs;
    }

    char* get(Key* key) {
        if (pairs->count(key) > 0) {
            return pairs->at(key);
        }
        else {
            return "";
        }
    }

    void put(Key* key, char* data) {
        if (pairs->count(key) > 0) {
            printf("Node already contains key: %s\n", key->name.c_str());
            printf("Did not add new value\n");
        }
        else {
            pairs->insert(make_pair<Key*, char*>((Key*)key, (char*)data));
        }
    }
};