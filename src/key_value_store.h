#pragma once

#include <string>
#include "dataframe.h"
#include "node.h"

class Key {
public:
    string name;
    
    Key(string s) {
        name = s;
    }

    ~Key() {
        delete name;
    }
};

class KVStore {
public:
    vector<Node> nodes;

    KVStore(vector<Node> lon) {
        nodes = lon;
    }

    
    
    DataFrame get(Key key) {

    }

    void put(Key key, DataFrame df) {
        //serialize the df into bytes
        //split up the byte array into (number of nodes) chunks
        //put into each array according to the node index, using the same key
    }
    
    DataFrame wait_and_get(Key key) {
        
    
};