#pragma once

#include <string>

using namespace std;

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