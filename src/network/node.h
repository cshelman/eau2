#pragma once

#include "key_value_store.h"
#include <unordered_map>
#include <string>
#include <stdio.h>

using namespace std;

class Node {
public:
    unordered_map<string, char*>* pairs;
    size_t id;

    Node(size_t node_id) {
        id = node_id;
        pairs = new unordered_map<string, char*>();
    }

    ~Node() {
        delete pairs;
    }

    char* get(Key* key) {
        if (pairs->count(key->name) > 0) {
            // printf("Node %ld contains pair: { `%s` , `%s` }\n", id, key->name.c_str(), pairs->at(key->name));
            return pairs->at(key->name);
        }
        else {
            // printf("Node %ld does not contain key: `%s`\n", id, key->name.c_str());
            return (char*)"";
        }
    }

    void put(Key* key, char* data) {
        if (pairs->count(key->name) > 0) {
            printf("Node already contains key: `%s`\n", key->name.c_str());
            printf("Did not add new value\n");
        }
        else {
            char* copy = new char[strlen(data) + 1];
            strcpy(copy, data);
            pairs->insert({key->name, copy});
            char* val = pairs->at(key->name);
        }
    }
};