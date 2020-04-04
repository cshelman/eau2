#pragma once

#include "../src/serializer/buffer.h"
#include "../src/serializer/serial.h"
#include "../src/dataframe/column.h"
#include "../src/dataframe/dataframe.h"

#define VEC_SIZE 1000000

void test_string() {
    String* s1 = new String("string one");
    string ss1 = serialize_string(s1);
    String* s2 = deserialize_string((char*)ss1.c_str());
    assert(s1->equals(s2));
    delete s1;
    delete s2;
}

void test_int() {
    int i1 = 123;
    string si1 = serialize_int(i1);
    int i2 = deserialize_int((char*)si1.c_str());
    assert(i1 == i2);
}

void test_float() {
    float f1 = 98.765;
    string sf1 = serialize_float(f1);
    float f2 = deserialize_float((char*)sf1.c_str());
    assert(f1 == f2);
}

void test_bool() {
    bool b1 = true;
    string sb1 = serialize_bool(b1);
    bool b2 = deserialize_bool((char*)sb1.c_str());
    assert(b1 == b2);
}

void test_str_vector() {
    vector<String*>* vs = new vector<String*>();
    for (int i = 0; i < VEC_SIZE; i++) {
        string str = "hiii";
        str += to_string(i);
        String* sss = new String(str.c_str());
        vs->push_back(sss);
    }
    string svs = serialize_str_vector(vs);
    vector<String*>* dvs = deserialize_str_vector((char*)svs.c_str());
    for (int i = 0; i < VEC_SIZE; i++) {
      assert(dvs->at(i)->equals(vs->at(i)));
    }

    delete vs;
    delete dvs;
}

void test_int_vector() {
    vector<int>* vi = new vector<int>();
    for (int i = 0; i < VEC_SIZE; i++) {
        vi->push_back(i);
    }
    string svi = serialize_int_vector(vi);
    vector<int>* dvi = deserialize_int_vector((char*)svi.c_str());
    for (int i = 0; i < VEC_SIZE; i++) {
      assert(dvi->at(i) == vi->at(i));
    }

    delete vi;
    delete dvi;
}

void test_float_vector() {
    vector<float>* vf = new vector<float>();
    for (int i = 0; i < VEC_SIZE; i++) {
        vf->push_back((float)i);
    }
    string svf = serialize_float_vector(vf);
    vector<float>* dvf = deserialize_float_vector((char*)svf.c_str());
    for (int i = 0; i < VEC_SIZE; i++) {
        assert(vf->at(i) == dvf->at(i));
    }

    delete vf;
    delete dvf;
}

void test_col_vector() {
    Schema* s = new Schema();
    s->add_column('I');
    s->add_column('F');
    s->add_column('B');
    s->add_column('S');

    DataFrame* df1 = new DataFrame(*s);

    for (size_t i = 0; i < VEC_SIZE; i++) {
        Row* row = new Row(df1->get_schema());
        row->set(0, (int)i);
        row->set(1, (float)i);
        row->set(2, (bool)(i % 2));
        String* str = new String("IM STR");
        row->set(3, str);
        df1->add_row(*row);
        delete str;
        delete row;
    }

    string str = serialize_col_vector(df1->col_arr);
    vector<Column*>* col_arr = deserialize_col_vector((char*)str.c_str());

    Schema* ns = new Schema();
    ns->num_rows_ = col_arr->at(0)->size();
    DataFrame* df2 = new DataFrame(*ns);
    for (int i = 0; i < col_arr->size(); i++) {
      df2->add_column(col_arr->at(i));
    }

    assert(df1->equals(df2));

    delete df1;
    delete df2;
    delete s;
    delete ns;
    delete col_arr;
}

void test_serialize() {
    // test primitives
    test_string();
    test_int();
    test_float();
    test_bool();

    // test primitive vectors
    test_str_vector();
    test_int_vector();
    test_float_vector();

    // test column vector
    test_col_vector();

    printf("serialize test: SUCCESS\n");
}