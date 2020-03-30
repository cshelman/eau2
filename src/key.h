#pragma once

#include <string>

class Key {
public:
    string name;
    
    Key(string s) {
        name = s;
    }

    Key* copy() {
        return new Key(name);
    }
};