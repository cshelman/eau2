#include "../src/network/client.h"

int main(int argc, char** argv) {
    char* addr = argv[1];
    char* serv_addr = argv[2];

    // printf("Starting client: %s, %s\n", addr, serv_addr);

    Client* c = new Client(addr, serv_addr);
    c->register_ip();
    c->be_client();

    return 0;
}