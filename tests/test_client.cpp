#include "../src/network/client.h"
#include "../src/rowers/word_count.h"

int main(int argc, char** argv) {
    char* addr = argv[1];
    char* serv_addr = argv[2];

    Client* c = new Client(addr, serv_addr);
    c->register_ip();
    c->be_client();

    delete c;
    return 0;
}