#pragma once

#include "server.h"
#include "../dataframe/column.h"
#include "../dataframe/row.h"
#include "../dataframe/schema.h"
#include "../dataframe/dataframe.h"
#include "../serializer/serial.h"
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

    Rower* apply(Rower* rower, Key* key) {
        if (pairs->count(key->name) > 0) {
            DataFrame* df = pairs->at(key->name);
            df->pmap(*rower);
            printf("node %ld applied rower\n", id);
            return rower;
        }
        else {
            printf("ACT: node %ld did not contain key: %s\n", id, (char*)key->name.c_str());
            return rower;
        }
    }

    char* get(Key* key) {
        if (pairs->count(key->name) > 0) {
            DataFrame* df = pairs->at(key->name);
            
            string* temp = new string(serialize_col_vector(df->col_arr));
            char* val = new char[temp->size() + 1];
            strcpy(val, temp->c_str());
            val[temp->size() + 1] = '\0';

            return val;
        }
        else {
            printf("GET: node did not contain key: %s\n", (char*)key->name.c_str());
            return (char*)"";
        }
    }

    void put(Key* key, char* data) {
        if (pairs->count(key->name) > 0) {
            printf("Node already contains key: `%s`\n", key->name.c_str());
            printf("Did not add new value\n");
        }
        else {
            // throw away message if empty
            if (strcmp(data, "") == 0) {
                printf("Node received blank PUT\n");
                return;
            }
            
            vector<Column*>* col_arr = deserialize_col_vector(data);

            Schema* schema = new Schema();
            schema->num_rows_ = col_arr->at(0)->size();
            DataFrame* df = new DataFrame(*schema);
            
            for (int i = 0; i < col_arr->size(); i++) {
                df->add_column(col_arr->at(i));
            }
            
            // printf("put key: %s\n", (char*)key->name.c_str());
            pairs->insert({key->name, df});
            // printf("done with put in node\n");
        }
    }
};