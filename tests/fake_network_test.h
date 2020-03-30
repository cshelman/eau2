#pragma once

#include "../src/node.h"
#include "../src/fake_network.h"
#include "../src/key_value_store.h"
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
        printf("\t%ld: receiving msg...\n", node_id);
        Message* recv = net->recv_msg(node_id);
        if (recv->type == MsgType::Put) {
            node.put(recv->key, recv->contents);
        }
        else if (recv->type == MsgType::Get) {
            node.get(recv->key);
        }
        else if (recv->type == MsgType::Kill) {
            return;
        }
        else {
            printf("Panic! Node received a message of invalid type\n");
            exit(1);
        }
    }
}

void test_fake_network(size_t nodes) {
    size_t num_nodes = nodes;
    printf("Creating network...\n");
    net = new FakeNetwork(num_nodes);
    printf("Creating KVStore...\n");
    KVStore* kv = new KVStore(net);

    printf("Creating threads...\n");
    thread* ts[num_nodes];
    for (int i = 0; i < num_nodes; i++) {
        ts[i] = new thread(fake_network_callback, i);
    }

    printf("Creating dataframe...\n");
    Schema* s = new Schema("IFBS");
    DataFrame* df1 = new DataFrame(*s);
    for (size_t i = 0; i < 10; i++) {
        Row* row = new Row(df1->get_schema());
        row->set(0, (int)i);
        row->set(1, (float)i);
        row->set(2, (bool)(i % 2));
        row->set(3, "non-changing :(");
        df1->add_row(*row);
        delete row;
    }

    printf("Creating Key...\n");
    Key* k1 = new Key("one");
    printf("Putting df1 in kv...\n");
    kv->put(k1, df1);
    printf("Getting df1 from kv...\n");
    DataFrame* df2 = kv->get(k1);
    printf("Comparing dfs kv...\n");
    assert(df1->col_arr == df2->col_arr);

    for (int i = 0; i < num_nodes; i++) {
        ts[i]->join();
    }
    delete df1;
    delete df2;
    delete k1;

    printf("fake network test: SUCCESS\n");
}