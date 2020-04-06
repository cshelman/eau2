#pragma once

#include "node.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <poll.h>
#include <thread>
#include <vector>
#include <algorithm>

#include <errno.h>

using namespace std;

#define BUFFER_SIZE 4096

class Client {
public:
    char* ip;
    char* port;
    char* address;
    const char* server_ip;
    const char* server_port;
    struct sockaddr_in client;
    int clientlen;
    int server_sock;
    Node* node;

    Client(char* address_in, char* server_address) {
        address = new char[strlen(address_in) + 1];
        strcpy(address, address_in);

        ip = strtok(address_in, ":");
        port = strtok(NULL, ":");

        client.sin_family = AF_INET;
        client.sin_port = htons(atoi(port));
        assert(inet_pton(AF_INET, ip, &client.sin_addr) > 0);
        clientlen = sizeof(client);

        server_ip = strtok(server_address, ":");
        server_port = strtok(NULL, ":");
    }

    ~Client() {
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

    void send_message(Key* key, char* contents) {
        Message* msg = new Message(MsgType::Put, key, contents);
        string serialized_msg = serialize_message(msg);
        char* encoded_msg = encode(serialized_msg);
        send(server_sock, encoded_msg, strlen(encoded_msg) + 1, 0);
    }

    vector<string>* decode(char* s, int bytes_read) {
        vector<string>* decoded_strings = new vector<string>();
        int start = 0;
        bool in_msg = false;
        int tic_counter = 0;

        for (int i = 0; i < bytes_read; i++) {
            if (s[i] == '`') {
                tic_counter ++;
                if (in_msg && tic_counter == 3) {
                    char* decoded = new char[i - start - 1];
                    memcpy(decoded, s + start, i - start - 2);
                    string str(decoded);
                    decoded_strings->push_back(str);
                    delete[] decoded;
                    tic_counter = 0;
                }
                else if (tic_counter == 3) {
                    start = i + 1;
                }
                in_msg = !in_msg;
            }
            else {
                tic_counter = 0;
            }
        }
        return decoded_strings;
    }

    // backticks are not a valid character to send
    char* encode(string s) {
        string* encoded_s = new string("```");
        encoded_s->append(s);
        encoded_s->append("```");
        return (char*)encoded_s->c_str();
    }

    void be_client() {
        char* node_id = new char[8];
        memset(node_id, '\0', 8);
        read(server_sock, node_id, BUFFER_SIZE);
        node = new Node(atoi(node_id));
       
        string* put_msg = new string("");
        while (true) {

            char* buffer = new char[BUFFER_SIZE];
            memset(buffer, '\0', BUFFER_SIZE);

            int bytes_read = recv(server_sock, buffer, BUFFER_SIZE, 0);
            // printf("just received\n");
            vector<string>* decoded_strings = decode(buffer, bytes_read);

            for (int i = 0; i < decoded_strings->size(); i++) {
                // printf("new string %s\n", (char*)decoded_strings->at(i).c_str());
                Message* msg = deserialize_message((char*)decoded_strings->at(i).c_str());
                if (msg->type == MsgType::Act) {
                    //does rower stuff
                }
                else if (msg->type == MsgType::Get) {
                    // printf("client recv GET msg\n");
                    char* df = node->get(msg->key);
                    // printf("sending\n");
                    send_message(msg->key, df);
                    // printf("DONNNE\n");
                }
                else if (msg->type == MsgType::Put) {
                    if (strcmp(msg->contents, "END") == 0) {
                        node->put(msg->key, (char*)put_msg->c_str());
                        *put_msg = "";
                    }
                    else {
                        put_msg->append(msg->contents);
                    }
                }
                else if (msg->type == MsgType::Kill) {
                    delete decoded_strings;
                    delete[] buffer;
                    return;
                }
                delete msg;
            }
            
            delete decoded_strings;
            delete[] buffer;
        }
    }
};