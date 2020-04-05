#pragma once

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <assert.h>
#include <poll.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>

#include <errno.h>

#include "vector.h"
#include "buffer.h"

#define BUFFER_SIZE 4096

class Server {
public:
    char* ip;
    char* port;
    Vector* client_ips;
    Vector* client_sockets;
    struct sockaddr_in serv;
    int servlen;
    int sock_fd;

    Server(char* address) {
        ip = strtok(address, ":");
        port = strtok(NULL, ":");
        client_ips = new Vector();
        client_sockets = new Vector();

        serv.sin_family = AF_INET;
        serv.sin_port = htons(atoi(port));
        assert(inet_pton(AF_INET, ip, &serv.sin_addr) > 0);
        servlen = sizeof(serv);
    }

    ~Server() {
        delete client_ips;
        delete client_sockets;
    }

    // backticks are not a valid character to send
    char* encode(char* s) {
        char* encoded_s = new char[strlen(s) + 3];
        strcpy(encoded_s, "`");
        encoded_s = strcat(encoded_s, s);
        encoded_s = strcat(encoded_s, "`");
        return encoded_s;
    }

    void startup() {
        int yes = 1;
        assert((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) > 0);
        assert(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &yes, sizeof(yes)) >= 0);
        assert(bind(sock_fd, (struct sockaddr*)&serv, servlen) >= 0);
        assert(listen(sock_fd, 10) >= 0);
    }

    void distribute_ips() {
        for (int i = 0; i < client_sockets->get_len(); i++) {
            for (int j = 0; j < client_ips->get_len(); j++) {
                char* encoded = encode(client_ips->get(j));
                int sent = send(atoi(client_sockets->get(i)), encoded, strlen(encoded) + 1, 0);
            }
        }
    }

    void registration() {
        while (true) {
            Buffer* buffer = new Buffer();
            int client_sock;
            char temp_buffer[BUFFER_SIZE];
            assert((client_sock = accept(sock_fd, (struct sockaddr*)&serv,(socklen_t*)&servlen)) >= 0);
            
            int status;
            assert((status = recv(client_sock, temp_buffer, BUFFER_SIZE, 0)) >= 0);
            buffer->add(temp_buffer);
            printf("client %s connected\n", buffer->val);

            client_ips->add(buffer->val);

            char num[16];
            sprintf(num, "%d", client_sock);
            client_sockets->add(num);

            distribute_ips();
        }
    }
};

class Client {
public:
    char* ip;
    char* port;
    char* address;
    const char* server_ip;
    const char* server_port;
    Vector* neighbor_ips;
    Vector* neighbor_sockets;
    std::thread** neighbor_threads;
    struct sockaddr_in client;
    int clientlen;
    int server_sock;

    Client(char* address_in, char* server_address) {
        address = new char[strlen(address_in) + 1];
        strcpy(address, address_in);

        ip = strtok(address_in, ":");
        port = strtok(NULL, ":");
        neighbor_ips = new Vector();
        neighbor_sockets = new Vector();

        client.sin_family = AF_INET;
        client.sin_port = htons(atoi(port));
        assert(inet_pton(AF_INET, ip, &client.sin_addr) > 0);
        clientlen = sizeof(client);

        server_ip = strtok(server_address, ":");
        server_port = strtok(NULL, ":");

        neighbor_threads = new std::thread*[512];
    }

    ~Client() {
        delete neighbor_ips;
        delete neighbor_sockets;
    }

    // create connection with server
    void register_ip() {
        struct sockaddr_in to_serv;
        assert((server_sock = socket(AF_INET, SOCK_STREAM, 0)) >= 0);
        to_serv.sin_family = AF_INET;
        to_serv.sin_port = htons(atoi(server_port));
        assert(inet_pton(AF_INET, server_ip, &to_serv.sin_addr) > 0);
        assert(connect(server_sock, (struct sockaddr*)&to_serv, sizeof(to_serv)) >= 0);

        int sent_bytes = 0;
        while (sent_bytes < sizeof(address) / sizeof(char)) {
            int ret;
            assert((ret = send(server_sock, address, strlen(address) + 1, 0)) >= 0);
            sent_bytes += ret;
        }
    }

    void send_message(char* buffer) {
        for (int i = 0; i < neighbor_sockets->get_len(); i++) {
            printf("sending %s to %s(%s)\n", buffer, neighbor_ips->get(i), neighbor_sockets->get(i));
            send(atoi(neighbor_sockets->get(i)), buffer, strlen(buffer) + 1, 0);
        }
    }

    Vector* decode(char* s, int bytes_read) {
        Vector* decoded_strings = new Vector();
        int start = 0;
        bool in_msg = false;

        for (int i = 0; i < bytes_read; i++) {
            if (s[i] == '`') {
                if (in_msg) {
                    char* decoded = new char[i - start + 1];
                    memcpy(decoded, s + start, i - start);
                    decoded_strings->add(decoded);
                    delete[] decoded;
                }
                else {
                    start = i + 1;
                }
                in_msg = !in_msg;
            }
        }
        return decoded_strings;
    }

    void create_neighbor_connection(char* neighbor_port, char* neighbor_ip) {
        // creating input socket
        struct sockaddr_in neigh;

        neigh.sin_family = AF_INET;
        neigh.sin_port = htons(atoi(neighbor_port));
        assert(inet_pton(AF_INET, ip, &neigh.sin_addr) > 0);
        int neighlen = sizeof(neigh);

        int yes = 1;
        int listen_sock;
        assert((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) > 0);
        assert(setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &yes, sizeof(yes)) >= 0);
        assert(bind(listen_sock, (struct sockaddr*)&neigh, neighlen) >= 0);
        assert(listen(listen_sock, 10) >= 0);

        int read_sock;
        assert((read_sock = accept(listen_sock, (struct sockaddr*)&neigh,(socklen_t*)&neighlen)) >= 0);

        
        while (true) {

            char* buffer = new char[BUFFER_SIZE];
            memset(buffer, '\0', BUFFER_SIZE);
            int bytes_read = read(read_sock, buffer, BUFFER_SIZE);
            Vector* decoded_strings = decode(buffer, bytes_read);

            for (int i = 0; i < decoded_strings->get_len(); i++) {
                printf("[%s]: %s\n", neighbor_ip, decoded_strings->get(i));
            }
            delete decoded_strings;
            delete[] buffer;
        }
    }

    // backticks are not a valid character to send
    char* encode(char* s) {
        char* encoded_s = new char[strlen(s) + 3];
        strcpy(encoded_s, "`");

        char parsed_s[strlen(s)];
        memcpy(parsed_s, &s[0], strlen(s) - 1);
        parsed_s[strlen(s) - 1] = '\0';

        encoded_s = strcat(encoded_s, parsed_s);
        encoded_s = strcat(encoded_s, "`");
        return encoded_s;
    }

    void communicate_with_neighbor() {
        while(true) {
            char* buffer = new char[BUFFER_SIZE];
            fgets(buffer, BUFFER_SIZE, stdin);

            for (int i = 0; i < neighbor_sockets->get_len(); i++) {
                char* encoded = encode(buffer);
                int sent = send(atoi(neighbor_sockets->get(i)), encoded, strlen(encoded) + 1, 0);
            }
        }
    }

    void be_client() {
        std::thread* communicate_thread = new std::thread(&Client::communicate_with_neighbor, this);
       
        while (true) {

            char* buffer = new char[BUFFER_SIZE];
            memset(buffer, '\0', BUFFER_SIZE);
            int bytes_read = read(server_sock, buffer, BUFFER_SIZE);
            Vector* decoded_strings = decode(buffer, bytes_read);

            for (int i = 0; i < decoded_strings->get_len(); i++) {

                if (!neighbor_ips->contains(decoded_strings->get(i)) && strcmp(decoded_strings->get(i), address) != 0) {

                    neighbor_ips->add(decoded_strings->get(i));
                    
                    char* cur_ip = strtok(decoded_strings->get(i), ":");
                    char* cur_port = strtok(NULL, ":");

                    // thread to listen
                    std::thread* neighbor_thread = new std::thread(&Client::create_neighbor_connection, this, cur_port, cur_ip);
                    
                    struct sockaddr_in to_neigh;
                    int neigh_sock;
                    assert((neigh_sock = socket(AF_INET, SOCK_STREAM, 0)) >= 0);
                    to_neigh.sin_family = AF_INET;
                    to_neigh.sin_port = htons(atoi(port));
                    assert(inet_pton(AF_INET, cur_ip, &to_neigh.sin_addr) > 0);

                    while (connect(neigh_sock, (struct sockaddr*)&to_neigh, sizeof(to_neigh)) < 0) {}

                    
                    char* temp = new char[8];
                    sprintf(temp, "%d", neigh_sock);
                    neighbor_sockets->add(temp);
                    printf("%s:%s has joined the network\n", cur_ip, cur_port);
                    delete[] temp;
                }
            }
            delete decoded_strings;
            delete[] buffer;
        }

        for (int i = 0; i < neighbor_ips->get_len(); i++) {
            neighbor_threads[i]->join();
        }

        for (int i = 0; i < neighbor_sockets->get_len(); i++) {
            close(atoi(neighbor_sockets->get(i)));
        }
    }
};