#include "../src/network/client.h"
#include "../src/rowers/word_count.h"

int main(int argc, char** argv) {
    char* addr = argv[1];
    char* serv_addr = argv[2];

    Client* c = new Client(addr, serv_addr);
    WordCountRower* wr = new WordCountRower();
    c->set_rower(wr);
    c->register_ip();
    c->be_client();

    delete wr;
    delete c;
    return 0;
}