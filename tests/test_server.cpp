
#include "../src/network/server.h"
#include "../src/network/network_server.h"
#include "../src/dataframe/dataframe.h"
#include "../src/dataframe/schema.h"
#include "word_count_test.h"

int main(int argc, char** argv) {
    NetworkServer* ns = new NetworkServer(argv[1], atoi(argv[2]));
    Server* s = new Server(ns);

    DataFrame* df = parse_file("../dict.txt");

    // char temp;
    // printf("PRESS X TO PUT...");
    // cin >> temp;
    // printf("\n");
    
    Key* k1 = new Key("one");
    s->put(k1, df);

    // printf("PRESS X TO COUNT...");
    // cin >> temp;
    // printf("\n");

    WordCountRower* wcr = new WordCountRower();
    wcr = dynamic_cast<WordCountRower*>(s->run_rower(k1, wcr));
    wcr->print();

    // printf("PRESS X TO SHUTDOWN...");
    // cin >> temp;
    // printf("\n");

    s->shutdown();

    delete k1;
    delete df;
    delete wcr;

    printf("network test: SUCCESS\n");
    return 0;
}