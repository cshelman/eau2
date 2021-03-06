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

//the distributed KV store of eau2, this runs on each client
class Node {
public:
    unordered_map<string, DataFrame*>* pairs;
    size_t id;

    Node(size_t node_id) {
        id = node_id;
        pairs = new unordered_map<string, DataFrame*>();
    }

    ~Node() {
        for (pair<string, DataFrame*> element : *pairs) {
            delete element.second;
        }
        delete pairs;
    }

    //runs a rower on the dataframe that corresponds to the given key
    void apply(Rower* rower, Key* key) {
        if (pairs->count(key->name) > 0) {
            DataFrame* df = pairs->at(key->name);
            df->pmap(*rower);
        }
        else {
            printf("ACT: node %ld did not contain key: %s\n", id, (char*)key->name.c_str());
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

            // printf("Starting node put\n");
            // printf("\tTrying to deserialize column vector\n");
            vector<Column*>* col_arr = deserialize_col_vector(data);

            //we receive a vector of columns from the client
            //the following code constructs a df from that vector
            Schema* schema = new Schema();
            schema->num_rows_ = col_arr->at(0)->size();
            DataFrame* df = new DataFrame(*schema);
            
            // printf("\tTrying to add cols to dataframe\n");
            for (int i = 0; i < col_arr->size(); i++) {
                df->add_column(col_arr->at(i));
            }
          
            // printf("\tTrying to insert to map\n");
            pairs->insert({key->name, df});

            // printf("\tTrying to delete col_arr\n");
            for (int i = 0; i < col_arr->size(); i++) {
                if (col_arr->at(i)->get_type() == 'I') {
                    delete col_arr->at(i)->as_int();
                }
                else if (col_arr->at(i)->get_type() == 'F') {
                    delete col_arr->at(i)->as_float();
                }
                else if (col_arr->at(i)->get_type() == 'B') {
                    delete col_arr->at(i)->as_bool();
                }
                else if (col_arr->at(i)->get_type() == 'S') {
                    delete col_arr->at(i)->as_string();
                }
                else {
                    printf("Invalid column type when deleting col_arr in node put\n");
                }
            }
            delete col_arr;
            delete schema;
            // printf("\tDone with node put\n");
        }
    }
};