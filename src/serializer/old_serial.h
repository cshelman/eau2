#pragma once

#include <stdio.h>
#include <string.h>

#include "buffer.h"

// Helpers
void add_tag(const char* tag, char* val, Buffer* buffer) {
    buffer->add(tag);
    buffer->add(":{");
    buffer->add(val);
    buffer->add("}");
}

// char* are not allowed to contain double quotes at the moment
char* get_word(char* s, char open, char close) {
    int bracket_count = 0;
    bool in_string = false;
    int start = -1;
    for (int i = 0; i < strlen(s); i++) {
        if (s[i] == '\"') {
            in_string = !in_string;
        }
        else if (s[i] == open && !in_string) {
            bracket_count++;
            if (start == -1) {
                start = i + 1;
            }
        }
        else if (s[i] == close && !in_string) {
            bracket_count--;
            if (bracket_count == 0) {
                char* word = new char[strlen(s) + 1];
                memcpy(word, &s[start], i - start);
                return word;
            }
        }
    }
    printf("invalid word: `%s`\n", s);
    exit(1);
}

vector<String*>* get_components(char* s) {
    vector<String*>* components = new vector<String*>();
    bool in_string = false;
    int bracket_count = 0;
    int start = 0;
    printf("NEW GET COMPONENTS\n");
    for (int i = 0; i < strlen(s); i++) {
        if (s[i] == '\"') {
            in_string = !in_string;
        }
        else if (s[i] == '{' && !in_string) {
            bracket_count++;
        }
        else if (s[i] == '}' && !in_string) {
            bracket_count--;
        }
        else if (s[i] == ',' && !in_string && bracket_count == 0) {
            printf("at end of component\n");
            char* comp = new char[i - start];
            memcpy(comp, &s[start], i - start);
            String* str = new String(comp, i - start);
            components->push_back(str);
            start = i + 1;
            // delete comp;
        }
        if (i >= strlen(s) - 1) {
            printf("at end of string\n");
            char* comp = new char[i - start];
            memcpy(comp, &s[start], i - start);
            String* str = new String(comp, i - start);
            components->push_back(str);
            delete comp;
        }
    }

    printf("COMPONENTS ---------- \n");
    for (int i = 0; i < components->size(); i++) {
        printf("%s\n", components->at(i)->c_str());
    }
    printf("\n");
    
    return components;
}

// Serialization / deserialization functions

void serialize_chars(char* s, Buffer* buffer) {
    char* ns = new char[strlen(s) + 3];
    memcpy(&ns[0], "\"", 1);
    memcpy(&ns[1], s, strlen(s));
    memcpy(&ns[strlen(s) + 1], "\"\0", 2);
    add_tag("char*", ns, buffer);
    //delete[] ns;
}

char* deserialize_chars(char* s) {
    char* quotes = get_word(s, '{', '}');
    char* removed = new char[strlen(s) - 2];
    memcpy(removed, &quotes[1], strlen(s) - 10);

    //delete quotes;
    return removed;
}

void serialize_size_t(size_t n, Buffer* buffer) {
    char* ns = new char[64];
    sprintf(ns, "%zu", n);
    add_tag("size_t", ns, buffer);
    //delete[] ns;
}

size_t deserialize_size_t(char* s) {
    char* word = get_word(s, '{', '}');
    size_t st = 0;
    sscanf(word, "%zu", &st);
    //delete word;
    return st;
}

void serialize_int(int n, Buffer* buffer) {
    char* ns = new char[16];
    sprintf(ns, "%d", n);
    add_tag("int", ns, buffer);
    //delete[] ns;
}

int deserialize_int(char* s) {
    char* word = get_word(s, '{', '}');
    int i = atoi(word);
    //delete word;
    return i;
}

void serialize_bool(bool n, Buffer* buffer) {
    // if(n == 0) {
    //     char* null = new char[4];
    //     strcpy(null, "NULL");
    //     add_tag("bool", null, buffer);
    //     delete null;
    //     return;
    // }

    char* ns = new char[16];
    sprintf(ns, "%d", n);
    add_tag("bool", ns, buffer);
    // delete[] ns;
}

bool deserialize_bool(char* s) {
    char* word = get_word(s, '{', '}');

    if(strcmp(word, "NULL") == 0) {
        //delete word;
        return NULL;
    }

    int i = atoi(word);
    //delete word;
    return i; 
}

void serialize_float(float f, Buffer* buffer) {
    // if(f == NULL) {
    //     char* null = new char[4];
    //     strcpy(null, "NULL");
    //     add_tag("float", null, buffer);
    //     delete null;
    //     return;
    // }

    char* ns = new char[512];
    sprintf(ns, "%f", f);
    add_tag("float", ns, buffer);
    // delete[] ns;
}

float deserialize_float(char* s) {
    char* word = get_word(s, '{', '}');

    if(strcmp(word, "NULL") == 0) {
        //delete word;
        return NULL;
    }

    float f = atof(word);
    //delete word;
    return f;
}

void serialize_string(String* s, Buffer* buffer) {
    if(s == nullptr) {
        char* null = new char[4];
        strcpy(null, "NULL");
        add_tag("string", null, buffer);
        delete null;
        return;
    }

    Buffer* temp = new Buffer();

    serialize_chars(s->c_str(), temp);
    temp->add(",");
    serialize_size_t(s->size(), temp);
    temp->add("\0");

    add_tag("string", temp->val, buffer);
    delete temp;
}

String* deserialize_string(char* s) {
    char* word = get_word(s, '{', '}');

    if(strcmp(word, "NULL") == 0) {
        //delete word;
        return nullptr;
    }

    vector<String*>* components = get_components(word);
    char* val = deserialize_chars(components->at(0)->c_str());
    int len = deserialize_size_t(components->at(1)->c_str());
    String* str = new String(val, len);

    // delete components;
    //delete word;
    // delete val;
    return str;
}

void serialize_string_vector(vector<String*>* vs, Buffer* buffer) {
    Buffer* temp = new Buffer();

    for(int i = 0; i < vs->size(); i++) {
        serialize_string(vs->at(i), temp);
        if(i < vs->size() - 1) {
            temp->add(",");
        }
    }

    add_tag("StringVector", temp->val, buffer);
    delete temp;
}

vector<String*>* deserialize_string_vector(char* s) {
    char* word = get_word(s, '{', '}');
    vector<String*>* components = get_components(word);
    vector<String*>* vs = new vector<String*>();

    for(int i = 0; i < components->size(); i++) {
        String* st = deserialize_string(components->at(i)->c_str());
        vs->push_back(st);
    }

    // delete components;
    //delete word;
    return vs;
}

void serialize_int_vector(vector<int>* vi, Buffer* buffer) {
    Buffer* temp = new Buffer();

    for(int i = 0; i < vi->size(); i++) {
        serialize_int(vi->at(i), temp);
        if(i < vi->size() - 1) {
            temp->add(",");
        }
    }

    add_tag("IntVector", temp->val, buffer);
    
    delete temp;
}

vector<int>* deserialize_int_vector(char* s) {
    char* word = get_word(s, '{', '}');
    vector<String*>* components = get_components(word);
    vector<int>* vi = new vector<int>();

    for(int i = 0; i < components->size(); i++) {
        int val = deserialize_int(components->at(i)->c_str());
        vi->push_back(val);
    }

    // delete components;
    //delete word;
    return vi;
}

void serialize_float_vector(vector<float>* vf, Buffer* buffer) {
    Buffer* temp = new Buffer();

    for(int i = 0; i < vf->size(); i++) {
        serialize_float(vf->at(i), temp);
        if(i < vf->size() - 1) {
            temp->add(",");
        }
    }

    add_tag("FloatVector", temp->val, buffer);
    delete temp;
}

vector<float>* deserialize_float_vector(char* s) {
    char* word = get_word(s, '{', '}');
    vector<String*>* components = get_components(word);
    vector<float>* vf = new vector<float>();

    for(int i = 0; i < components->size(); i++) {
        float f = deserialize_float(components->at(i)->c_str());
        vf->push_back(f);
    }

    // delete components;
    //delete word;
    return vf;
}

void serialize_bool_vector(vector<bool>* vb, Buffer* buffer) {
    Buffer* temp = new Buffer();

    for(int i = 0; i < vb->size(); i++) {
        serialize_bool(vb->at(i), temp);
        if(i < vb->size() - 1) {
            temp->add(",");
        }
    }

    add_tag("BoolVector", temp->val, buffer);
    delete temp;
}

vector<bool>* deserialize_bool_vector(char* s) {
    char* word = get_word(s, '{', '}');
    vector<String*>* components = get_components(word);
    vector<bool>* vb = new vector<bool>();

    for(int i = 0; i < components->size(); i++) {
        bool b = deserialize_bool(components->at(i)->c_str());
        vb->push_back(b);
    }

    // delete components;
    //delete word;
    return vb;
}

void serialize_column(Column* c, Buffer* buffer) {
    Buffer* temp = new Buffer();

    if(c->get_type() == 'I') {
        serialize_int_vector(dynamic_cast<IntColumn*>(c)->arr, temp);
    } else if(c->get_type() == 'F') {
        serialize_float_vector(dynamic_cast<FloatColumn*>(c)->arr, temp);
    } else if(c->get_type() == 'S') {
        serialize_string_vector(dynamic_cast<StringColumn*>(c)->arr, temp);
    } else if(c->get_type() == 'B') {
        serialize_bool_vector(dynamic_cast<BoolColumn*>(c)->arr, temp);
    }

    add_tag("Column", temp->val, buffer);
    delete temp;
}

Column* deserialize_column(char* s) {
    char* col_str = get_word(s, '{', '}');

    char type[strlen(col_str)];
    char *colon;
    strcpy(type, col_str);
    colon = strchr(type, ':');
    if (colon != 0)
        *colon = 0;

    if(strcmp(type, "IntVector") == 0) {
        IntColumn* ic = new IntColumn();
        vector<int>* vi = deserialize_int_vector(col_str);
        ic->arr = vi;
        // delete col_str;
        return ic;
    } else if(strcmp(type, "FloatVector") == 0) {
        FloatColumn* fc = new FloatColumn();
        vector<float>* vf = deserialize_float_vector(col_str);
        fc->arr = vf;
        // delete col_str;
        return fc;
    } else if(strcmp(type, "BoolVector") == 0) {
        BoolColumn* bc = new BoolColumn();
        vector<bool>* vb = deserialize_bool_vector(col_str);
        bc->arr = vb;
        // delete col_str;
        return bc;
    } else if(strcmp(type, "StringVector") == 0) {
        StringColumn* sc = new StringColumn();
        vector<String*>* vs = deserialize_string_vector(col_str);
        sc->arr = vs;
        // delete col_str;
        return sc;
    }
}

void serialize_col_vector(vector<Column*>* vc, Buffer* buffer) {
    Buffer* temp = new Buffer();

    for(int i = 0; i < vc->size(); i++) {
        serialize_column(vc->at(i), temp);
        if(i < vc->size() - 1) {
            temp->add(",");
        }
    }

    add_tag("ColumnVector", temp->val, buffer);
    delete temp;
}

vector<Column*>* deserialize_col_vector(char* s) {
    printf("%s\n\n\n\n", s);
    char* word = get_word(s, '{', '}');
    vector<String*>* components = get_components(word);
    vector<Column*>* vc = new vector<Column*>();

    for(int i = 0; i < components->size(); i++) {
        printf("col %d serialized: %s\n\n", i, components->at(i)->c_str());
        Column* c = deserialize_column(components->at(i)->c_str());
        vc->push_back(c);
    }

    // delete word;
    // delete components;
    return vc;
}

void serialize_schema(Schema* s, Buffer* buffer) {
    Buffer* temp = new Buffer();

    serialize_size_t(s->num_cols_, temp);
    temp->add(",");

    serialize_size_t(s->num_rows_, temp);
    temp->add(",");

    serialize_string_vector(s->types_, temp);
    temp->add(",");

    serialize_string_vector(s->col_names_, temp);
    temp->add(",");

    serialize_string_vector(s->row_names_, temp);
    
    
    add_tag("Schema", temp->val, buffer);
    delete temp;
}

Schema* deserialize_schema(char* s) {
    char* word = get_word(s, '{', '}');
    vector<String*>* components = get_components(word);

    size_t num_cols = deserialize_size_t(components->at(0)->c_str());
    size_t num_rows = deserialize_size_t(components->at(1)->c_str());

    vector<String*>* types = deserialize_string_vector(components->at(2)->c_str());
    vector<String*>* col_names = deserialize_string_vector(components->at(3)->c_str());
    vector<String*>* row_names = deserialize_string_vector(components->at(4)->c_str());

    Schema* sc = new Schema();

    sc->num_cols_ = num_cols;
    sc->num_rows_ = num_rows;
    sc->types_ = types;
    sc->col_names_ = col_names;
    sc->row_names_ = row_names;

    // delete components;
    //delete word;
    return sc;
}

void serialize_dataframe(DataFrame* df, Buffer* buffer) {
    Buffer* temp = new Buffer();

    serialize_schema(df->schema_, temp);
    temp->add(",");

    serialize_col_vector(df->col_arr, temp);
    
    add_tag("DataFrame", temp->val, buffer);

    //printf("serialized dataframe: %s\n", buffer->val);
    delete temp;
}

DataFrame* deserialize_dataframe(char* s) {
    // printf("start deserializing dataframe:\n`%s`\n\n", s);
    char* word = get_word(s, '{', '}');
    vector<String*>* components = get_components(word);

    // printf("start deserializing shcema\n");
    Schema* sc = deserialize_schema(components->at(0)->c_str());
    // printf("done deserializing shcema\n");
    vector<Column*>* vc = deserialize_col_vector(components->at(1)->c_str());
    
    DataFrame* df = new DataFrame(*sc);
    delete df->col_arr;
    df->col_arr = vc;
    
    //delete word;
    // delete components;
    return df;
}

