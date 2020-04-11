#pragma once

#include "../src/serializer/serial.h"
#include "../src/dataframe/column.h"
#include "../src/dataframe/dataframe.h"
#include "../src/network/key.h"
#include "../src/network/message.h"
#include "../src/rowers/find_projects.h"
#include "../src/rowers/find_users.h"
#include <string.h>
#include <bitset>

#define VEC_SIZE 10000

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

void test_message() {
    Key* k1 = new Key("key_name");
    Message* m1 = new Message(MsgType::Get, k1, (char*)"different contents now 123123123", -1);
    string sm1 = serialize_message(m1);
    Message* m2 = deserialize_message((char*)sm1.c_str());

    assert(m1->type == m2->type);
    assert(m1->key->name == m2->key->name);
    assert(strcmp(m1->contents, m2->contents) == 0);

    delete k1;
    delete m1;
    delete m2;
}

void test_projects_rower() {
    bool bs[9];
    bs[0] = true;
    bs[1] = true;
    bs[2] = true;
    bs[3] = true;
    bs[4] = false;
    bs[5] = false;
    bs[6] = false;
    bs[7] = true;
    bs[8] = true;

    bool ps[6];
    ps[0] = true;
    ps[1] = false;
    ps[2] = true;
    ps[3] = false;
    ps[4] = true;
    ps[5] = true;

    FindProjectsRower* fpr = new FindProjectsRower(bs, 9, 6);
    fpr->project_ids = ps;
    char* sr = fpr->serialize();
    // string* spr = new string(sr, 21);
    // char* cs = (char*)spr->c_str();
    // for(int i = 0; i < 24; i++) {
    //     char c = cs[i];
    //     printf("char: %c\n", c);
    //     bitset<8> b1(c);
    //     cout << b1 << endl;
    //     char cc = sr[i];
    //     printf("char2: %c\n", cc);
    //     bitset<8> b2(cc);
    //     cout << b2 << endl;
    // }
    

    // FindProjectsRower* nfpr = new FindProjectsRower((char*)spr->c_str());
    FindProjectsRower* nfpr = new FindProjectsRower(sr);
    assert(fpr->size == nfpr->size);
    assert(fpr->p_size == nfpr->p_size);
    for (int i = 0; i < nfpr->size; i++) {
        assert(fpr->user_ids[i] == nfpr->user_ids[i]);
    }
    for (int i = 0; i < nfpr->p_size; i++) {
        printf("%d | %d\n", fpr->project_ids[i], nfpr->project_ids[i]);
        assert(fpr->project_ids[i] == nfpr->project_ids[i]);
    }

    printf("PASSED\n");

    delete sr;
    // delete spr;
    delete fpr;
    delete nfpr;
}

void test_users_rower() {
    bool bs[9];
    bs[0] = true;
    bs[1] = true;
    bs[2] = true;
    bs[3] = true;
    bs[4] = false;
    bs[5] = false;
    bs[6] = false;
    bs[7] = true;
    bs[8] = true;

    bool us[6];
    us[0] = true;
    us[1] = false;
    us[2] = true;
    us[3] = false;
    us[4] = true;
    us[5] = true;

    FindUsersRower* fur = new FindUsersRower(bs, 9, 6);
    fur->user_ids = us;
    char* sr = fur->serialize();
    string* ssr = new string(sr, 21);

    printf("%s\n", sr);
    printf("%s\n", (char*)ssr->c_str());
    printf("size: %ld\n", strlen(sr));
    assert(memcmp(sr, (char*)ssr->c_str(), 21) == 0);

    // char* sr2 = (char*)ssr->c_str();

    FindUsersRower* nfur = new FindUsersRower(ssr->c_str());
    // FindUsersRower* nfur = new FindUsersRower(sr);
    assert(fur->size == nfur->size);
    assert(fur->u_size == nfur->u_size);
    for (int i = 0; i < nfur->size; i++) {
        assert(fur->project_ids[i] == nfur->project_ids[i]);
    }
    for (int i = 0; i < nfur->u_size; i++) {
        printf("%d | %d\n", fur->user_ids[i], nfur->user_ids[i]);
        assert(fur->user_ids[i] == nfur->user_ids[i]);
    }

    delete sr;
    // delete ssr;
    delete fur;
    delete nfur;
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

    // test messages
    test_message();

    // test rowers
    test_projects_rower();
    test_users_rower();

    printf("serialize test: SUCCESS\n");
}