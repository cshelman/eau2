#pragma once

#include <stdio.h>
#include <string.h>
#include <string>

#include "buffer.h"

using namespace std;

// serialize column vector
// serialize vectors of string, int, float, bool
// serialize string, int, float, bool

// COLUMN:
// {val1, val2, val3}
// {{val1, val2, val3}, {val1, val2, val3}, {val1, val2, val3}}

string serialize_int(int i) {
    return to_string(i);
}

int deserialize_int(char* s) {
    return atoi(s);
}

string serialize_bool(bool b) {
    return to_string(b);
}

bool deserialize_bool(char* s) {
    return atoi(s);
}

string serialize_float(float i) {
    return to_string(i);
}

float deserialize_float(char* s) {
    return atof(s);
}

string serialize_string(String* s) {
    return string(s->c_str());
}

String* deserialize_string(char* s) {
    String* str = new String(s);
    return str;
}

string serialize_str_vector(vector<String*>* vs) {
    string serialized_vector = "{";
    for (int i = 0; i < vs->size(); i++) {
        string serialized_str = serialize_string(vs->at(i));
        serialized_vector += '`' + serialized_str + '`';
        if (i < vs->size() - 1) {
            serialized_vector += ",";
        }
    }
    serialized_vector += "}";
    return serialized_vector;
}

vector<String*>* deserialize_str_vector(char* s) {
    string* str = new string(s);
    vector<String*>* vs = new vector<String*>();
    int prev_pos = 1;
    int pos = 0;
    while (pos != string::npos) {
        pos = str->find("`,", prev_pos);
        string token;
        if (pos == string::npos) {
            token = str->substr(prev_pos + 1, str->size() - prev_pos - 3);
        }
        else {
            token = str->substr(prev_pos + 1, pos - prev_pos - 1);
            prev_pos = pos + 2;
        }
        String* temp = deserialize_string((char*)token.c_str());
        vs->push_back(temp);
    }
    return vs;
}

string serialize_int_vector(vector<int>* vi) {
    string serialized_vector = "{";
    for (int i = 0; i < vi->size(); i++) {
        string serialized_int = serialize_int(vi->at(i));
        serialized_vector += serialized_int;
        if (i < vi->size() - 1) {
            serialized_vector += ",";
        }
    }
    serialized_vector += "}";
    return serialized_vector;
}

vector<int>* deserialize_int_vector(char* s) {
    string* str = new string(s);
    vector<int>* vi = new vector<int>();
    int prev_pos = 1;
    int pos = 0;
    while (pos != string::npos) {
        pos = str->find(",", prev_pos);
        string token;
        if (pos == string::npos) {
            token = str->substr(prev_pos, str->size() - prev_pos - 1);
        }
        else {
            token = str->substr(prev_pos, pos - prev_pos);
            prev_pos = pos + 1;
        }

        int temp = deserialize_int((char*)token.c_str());
        vi->push_back(temp);
    }
    return vi;
}

string serialize_bool_vector(vector<bool>* vb) {
    string serialized_vector = "{";
    for (int i = 0; i < vb->size(); i++) {
        string serialized_bool = serialize_int(vb->at(i));
        serialized_vector += serialized_bool;
        if (i < vb->size() - 1) {
            serialized_vector += ",";
        }
    }
    serialized_vector += "}";
    return serialized_vector;
}

vector<bool>* deserialize_bool_vector(char* s) {
    string* str = new string(s);
    vector<bool>* vb = new vector<bool>();
    int prev_pos = 1;
    int pos = 0;
    while (pos != string::npos) {
        pos = str->find(",", prev_pos);
        string token;
        if (pos == string::npos) {
            token = str->substr(prev_pos, str->size() - prev_pos - 1);
        }
        else {
            token = str->substr(prev_pos, pos - prev_pos);
            prev_pos = pos + 1;
        }

        bool temp = deserialize_bool((char*)token.c_str());
        vb->push_back(temp);
    }
    return vb;
}

string serialize_float_vector(vector<float>* vf) {
    string serialized_vector = "{";
    for (int i = 0; i < vf->size(); i++) {
        string serialized_float = serialize_float(vf->at(i));
        serialized_vector += serialized_float;
        if (i < vf->size() - 1) {
            serialized_vector += ",";
        }
    }
    serialized_vector += "}";
    return serialized_vector;
}

vector<float>* deserialize_float_vector(char* s) {
    string* str = new string(s);
    vector<float>* vf = new vector<float>();
    int prev_pos = 1;
    int pos = 0;
    while (pos != string::npos) {
        pos = str->find(",", prev_pos);
        string token;
        if (pos == string::npos) {
            token = str->substr(prev_pos, str->size() - prev_pos - 1);
        }
        else {
            token = str->substr(prev_pos, pos - prev_pos);
            prev_pos = pos + 1;
        }

        float temp = deserialize_float((char*)token.c_str());
        vf->push_back(temp);
    }
    return vf;
}

string serialize_col_vector(vector<Column*>* vc) {

    string* serialized_vector = new string("{");
    
    for (int i = 0; i < vc->size(); i++) {
        Column* c = vc->at(i);
        if(c->get_type() == 'I') {
            *serialized_vector += "`I`:";
            string sv = serialize_int_vector(dynamic_cast<IntColumn*>(c)->arr);
            *serialized_vector += sv;
        } else if(c->get_type() == 'F') {
            *serialized_vector += "`F`:";
            string sv = serialize_float_vector(dynamic_cast<FloatColumn*>(c)->arr);
            *serialized_vector += sv;
        } else if(c->get_type() == 'S') {
            *serialized_vector += "`S`:";
            string sv = serialize_str_vector(dynamic_cast<StringColumn*>(c)->arr);
            *serialized_vector += sv;
        } else if(c->get_type() == 'B') {
            *serialized_vector += "`B`:";
            string sv = serialize_bool_vector(dynamic_cast<BoolColumn*>(c)->arr);
            *serialized_vector += sv;
        }

        if (i < vc->size() - 1) {
            *serialized_vector += ",";
        }
    }
    *serialized_vector += "}";
    return *serialized_vector;
}

vector<Column*>* deserialize_col_vector(char* s) {

    string* str = new string(s);
    vector<Column*>* vc = new vector<Column*>();
    int prev_pos = 1;
    int pos = 0;
    while (pos != string::npos) {
        pos = str->find("},", prev_pos);
        string token;
        if (pos == string::npos) {
            token = str->substr(prev_pos + 1, str->size() - prev_pos - 2);
        }
        else {
            token = str->substr(prev_pos + 1, pos - prev_pos);
            prev_pos = pos + 2;
        }

        string type = token.substr(0, 1);
        
        if (type == "I") {
            vector<int>* vi = deserialize_int_vector((char*)token.substr(3, token.size() - 3).c_str());
            IntColumn* ic = new IntColumn();
            ic->arr = vi;
            vc->push_back(ic);
        } else if (type == "F") {
            vector<float>* vf = deserialize_float_vector((char*)token.substr(3, token.size() - 3).c_str());
            FloatColumn* fc = new FloatColumn();
            fc->arr = vf;
            vc->push_back(fc);
        } else if (type == "S") {
            vector<String*>* vs = deserialize_str_vector((char*)token.substr(3, token.size() - 3).c_str());
            StringColumn* sc = new StringColumn();
            sc->arr = vs;
            vc->push_back(sc);
        } else if (type == "B") {
            vector<bool>* vb = deserialize_bool_vector((char*)token.substr(3, token.size() - 3).c_str());
            BoolColumn* bc = new BoolColumn();
            bc->arr = vb;
            vc->push_back(bc);
        }
    }
    return vc;
}