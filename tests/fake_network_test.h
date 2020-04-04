#pragma once

#include "../src/dataframe/dataframe.h"
#include "../src/network/node.h"
#include "../src/network/fake_network.h"
#include "../src/network/server.h"
#include <thread>
#include <vector>
#include <stdio.h>

using namespace std;

FakeNetwork* net;

// The code that runs the node
// This will effectively be identical in the real network version
void fake_network_callback(size_t node_id) {
    Node node(node_id);
    while (true) {
        Message* recv = net->recv_msg(node_id);

        if (recv == nullptr) {
            continue;
        }

        if (recv->type == MsgType::Put) {
            // printf("node %ld received: %s\n\n", node_id, recv->contents);
            node.put(recv->key, recv->contents);
        }
        else if (recv->type == MsgType::Get) {
            char* contents = node.get(recv->key);
            // printf("node sending back contents: `%s`\n", contents);
            Message* ret_msg = new Message(MsgType::Put, recv->key, contents);
            net->send_master(ret_msg);
            delete ret_msg;
        }
        else if (recv->type == MsgType::Kill) {
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
    net = new FakeNetwork(num_nodes);
    Server* kv = new Server(net);

    Schema* s = new Schema();
    s->add_column('I');
    s->add_column('F');
    s->add_column('B');
    s->add_column('S');

    DataFrame* df1 = new DataFrame(*s);

    for (size_t i = 0; i < 10000; i++) {
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

    Key* k1 = new Key("one");

    thread* ts[num_nodes];
    for (int i = 0; i < num_nodes; i++) {
        ts[i] = new thread(fake_network_callback, i);
    }

    kv->put(k1, df1);
    DataFrame* df2 = kv->get(k1);

    kv->shutdown();

    for (int i = 0; i < num_nodes; i++) {
        ts[i]->join();
    }
    
    assert(df1->equals(df2));

    delete df1;
    delete df2;
    delete k1;
    delete kv;

    printf("fake network test: SUCCESS\n");
}