
#include "../src/network/server.h"
#include "../src/network/network_server.h"
#include "../src/dataframe/dataframe.h"
#include "../src/dataframe/schema.h"
#include "word_count_test.h"

int main(int argc, char** argv) {
    NetworkServer* ns = new NetworkServer(argv[1], atoi(argv[2]));
    Server* s = new Server(ns);
    printf("server created\n");

    DataFrame* df = parse_file("data/100k.txt");
    printf("df created\n");
    
    Key* k1 = new Key("one");
    s->put(k1, df);
    printf("df put\n");
    s->run_rower(k1);
    printf("word count run\n");

    s->shutdown();

    delete k1;
    delete df;

    // Schema* sc = new Schema();
    // sc->add_column('I');
    // sc->add_column('F');
    // sc->add_column('B');
    // sc->add_column('S');

    // DataFrame* df1 = new DataFrame(*sc);

    // for (size_t i = 0; i < 10; i++) {
    //     Row* row = new Row(df1->get_schema());
    //     row->set(0, (int)i);
    //     row->set(1, (float)i);
    //     row->set(2, (bool)(i % 2));
    //     String* str = new String("IM STR");
    //     row->set(3, str);
    //     df1->add_row(*row);
    //     delete str;
    //     delete row;
    // }

    // Key* k1 = new Key("one");

    // s->put(k1, df1);
    // DataFrame* df2 = s->get(k1);
    // s->shutdown();

    // df2->print();

    // assert(df1->equals(df2));

    // delete k1;
    // delete df1;
    // delete df2;

    printf("network test: SUCCESS\n");
    return 0;
}