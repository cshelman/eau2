#pragma once

#include "../src/network/node.h"
#include "../src/network/fake_network.h"
#include "../src/network/key_value_store.h"
#include <thread>
#include <vector>
#include <stdio.h>

using namespace std;

FakeNetwork* net;

void fake_network_callback(size_t node_id) {
    printf("\t%ld: creating node...\n", node_id);
    Node node(node_id);
    printf("\t%ld: starting receive loop...\n", node_id);
    while (true) {
        // printf("\t%ld: receiving msg...\n", node_id);
        Message* recv = net->recv_msg(node_id);

        // printf("\t%ld: checking msg validity...\n", node_id);
        if (recv == nullptr) {
            // printf("\t%ld: null msg...\n", node_id);
            continue;
        }

        // printf("\t%ld: checking msg type...\n", node_id);
        if (recv->type == MsgType::Put) {
            printf("-\t%ld: Put msg...\n", node_id);
            node.put(recv->key, recv->contents);
        }
        else if (recv->type == MsgType::Get) {
            printf("-\t%ld: Get msg...\n", node_id);
            char* contents = node.get(recv->key);
            Message* ret_msg = new Message(MsgType::Put, recv->key, contents);
            net->send_master(ret_msg);
            delete ret_msg;
        }
        else if (recv->type == MsgType::Kill) {
            printf("-\t%ld: Kill msg...\n", node_id);
            delete recv;
            return;
        }
        else {
            printf("Panic! Node received a message of invalid type\n");
            exit(1);
        }
        delete recv;
    }
}

void test_fake_network(size_t nodes) {
    size_t num_nodes = nodes;
    printf("Creating network...\n");
    net = new FakeNetwork(num_nodes);
    printf("Creating KVStore...\n");
    KVStore* kv = new KVStore(net);

    printf("Creating schema...\n");
    Schema* s = new Schema();
    s->add_column('I', nullptr);
    s->add_column('F', nullptr);
    s->add_column('B', nullptr);
    s->add_column('S', nullptr);

    printf("Creating dataframe...\n");
    DataFrame* df1 = new DataFrame(*s);

    printf("Filling dataframe...\n");
    for (size_t i = 0; i < 10; i++) {
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

    printf("Creating Key...\n");
    Key* k1 = new Key("one");

    printf("Creating threads...\n");
    thread* ts[num_nodes];
    for (int i = 0; i < num_nodes; i++) {
        ts[i] = new thread(fake_network_callback, i);
    }

    printf("Putting df1 in kv...\n");
    kv->put(k1, df1);
    printf("Getting df1 from kv...\n");
    DataFrame* df2 = kv->get(k1);
    printf("Shutting down KVStore...\n");
    kv->shutdown();

    printf("Joining threads...\n");
    for (int i = 0; i < num_nodes; i++) {
        ts[i]->join();
    }
    
    printf("Comparing dataframes...\n");
    assert(df1->col_arr == df2->col_arr);

    delete df1;
    delete df2;
    delete k1;

    printf("fake network test: SUCCESS\n");
}