#pragma once

#include <vector>
#include "dataframe.h"

class Key : public Object {
public:
    const char* name;
    size_t node;

    Key(const char* name, size_t node) {
        this->name = name;
        this->node = node;
    }
}

class KV_store : public Object {
public:
    Vector<Node> nodes;
    

    dataframe get(Key k) {

    }

    dataframe put(Key k, dataframe df) {

    }

    dataframe wait_and_get(Key k) {
        
    }
}