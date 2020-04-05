#pragma once

#include "server.h"
#include "../dataframe/column.h"
#include "../dataframe/schema.h"
#include "../dataframe/dataframe.h"
#include "../serializer/serial.h"
#include "../serializer/buffer.h"
#include <unordered_map>
#include <string>
#include <stdio.h>

using namespace std;

class Node {
public:
    unordered_map<string, DataFrame*>* pairs;
    size_t id;

    Node(size_t node_id) {
        id = node_id;
        pairs = new unordered_map<string, DataFrame*>();
    }

    ~Node() {
        delete pairs;
    }

    char* get(Key* key) {
        if (pairs->count(key->name) > 0) {
            DataFrame* df = pairs->at(key->name);
            string* temp = new string(serialize_col_vector(df->col_arr));
            char* val = new char[temp->size() + 1];
            strcpy(val, temp->c_str());
            val[temp->size() + 1] = '\0';

            delete df;
            // printf("%ld: returning: `%s`\n", id, val);
            return val;
        }
        else {
            // printf("%ld: returning: ``\n", id);
            return (char*)"";
        }
    }

    void put(Key* key, char* data) {

        //printf("put in %s: \n", data);

        if (pairs->count(key->name) > 0) {
            printf("Node already contains key: `%s`\n", key->name.c_str());
            printf("Did not add new value\n");
        }
        else {
            vector<Column*>* col_arr = deserialize_col_vector(data);

            Schema* schema = new Schema();
            DataFrame* df = new DataFrame(*schema);
            
            for (int i = 0; i < col_arr->size(); i++) {
                df->add_column(col_arr->at(i));
            }
            
            pairs->insert({key->name, df});
        }
    }
};