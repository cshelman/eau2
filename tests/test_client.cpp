#include "../src/network/client.h"
#include "../src/rowers/word_count.h"

int main(int argc, char** argv) {
    char* addr = argv[1];
    char* serv_addr = argv[2];

    // printf("Starting client: %s, %s\n", addr, serv_addr);

    Client* c = new Client(addr, serv_addr);
    WordCountRower* wr = new WordCountRower();
    c->set_rower(wr);
    c->register_ip();
    c->be_client();

    return 0;
}