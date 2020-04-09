#pragma once

#include "node.h"
#include "../rowers/word_count.h"

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
#include <sstream>
#include <iomanip>

#include <errno.h>

using namespace std;

#define CLIENT_BUF_SIZE 8192

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
    Rower* rower;

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

    void set_rower(Rower* r) {
        rower = r;
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
        printf("client sending: %s\n", encoded_msg);
        send(server_sock, encoded_msg, strlen(encoded_msg) + 1, 0);
    }

    vector<string>* decode(char* s, int bytes_read) {
        // printf("\ntrying to decode:\n%s\n", s);
        vector<string>* decoded_strings = new vector<string>();
        int start = 0;
        bool in_msg = false;
        int tic_counter = 0;

        for (int i = 0; i < bytes_read; i++) {
            if (s[i] == '`') {
                tic_counter ++;
                if (in_msg && tic_counter == 3) {
                    // printf("adding to vector\n");
                    char* decoded = new char[i - start - 2];
                    memcpy(decoded, s + start, i - start - 2);
                    // printf("decoded: %s\n", decoded);
                    string* str = new string(decoded);
                    decoded_strings->push_back(*str);
                    // printf("added: %s\n", (char*)str->c_str());
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
        stringstream ss;
        ss << setw(4) << setfill('0') << (s.size() + 1);
        string* result = new string(ss.str());
        result->append(s);

        return (char*)result->c_str();
    }

    void be_client() {
        char* node_id = new char[8];
        memset(node_id, '\0', 8);
        read(server_sock, node_id, CLIENT_BUF_SIZE);
        node = new Node(atoi(node_id));
        printf("made node #%s\n", node_id);
       
        string* put_msg = new string("");
        while (true) {
            char* size = new char[4];

            recv(server_sock, size, 4, 0);
            int bytes_read = 0;
            int bytes_to_read = atoi(size);
            char* buffer = new char[bytes_to_read];
            memset(buffer, '\0', bytes_to_read);

            while (bytes_read < bytes_to_read) {
                bytes_read += recv(server_sock, buffer, bytes_to_read - bytes_read, 0);
            }

            // printf("\n%s\n", buffer);
            Message* msg = deserialize_message(buffer);
            if (msg->type == MsgType::Act) {
                node->apply(rower, msg->key);
                // printf("Word count on %ld:\n", node->id);
                WordCountRower* r = dynamic_cast<WordCountRower*>(rower);
                r->print();
            }
            else if (msg->type == MsgType::Get) {
                // printf("client recv GET msg\n");
                char* df = node->get(msg->key);
                // printf("sending\n");
                send_message(msg->key, df);
                // printf("DONNNE\n");
            }
            else if (msg->type == MsgType::Put) {
                // printf("cmp: %s, \"END\"\n", msg->contents);
                if (strcmp(msg->contents, "END") == 0) {
                    printf("end of msg: %s\n", (char*)put_msg->c_str());
                    node->put(msg->key, (char*)put_msg->c_str());
                    *put_msg = "";
                }
                else {
                    put_msg->append(msg->contents);
                }
            }
            else if (msg->type == MsgType::Kill) {
                // delete decoded_strings;
                delete[] buffer;
                return;
            }
            delete msg;

            // printf("client recv: %s\n", buffer);

            // vector<string>* decoded_strings = decode(buffer, bytes_read);

            // for (int i = 0; i < decoded_strings->size(); i++) {
            //     // printf("\nclient working on: %s\n", (char*)decoded_strings->at(i).c_str());
            //     Message* msg = deserialize_message((char*)decoded_strings->at(i).c_str());
            //     if (msg->type == MsgType::Act) {
            //         node->apply(rower, msg->key);
            //         // printf("Word count on %ld:\n", node->id);
            //         WordCountRower* r = dynamic_cast<WordCountRower*>(rower);
            //         r->print();
            //     }
            //     else if (msg->type == MsgType::Get) {
            //         // printf("client recv GET msg\n");
            //         char* df = node->get(msg->key);
            //         // printf("sending\n");
            //         send_message(msg->key, df);
            //         // printf("DONNNE\n");
            //     }
            //     else if (msg->type == MsgType::Put) {
            //         if (strcmp(msg->contents, "END") == 0) {
            //             // printf("end of msg, key: %s\n", (char*)msg->key->name.c_str());
            //             node->put(msg->key, (char*)put_msg->c_str());
            //             *put_msg = "";
            //         }
            //         else {
            //             put_msg->append(msg->contents);
            //         }
            //     }
            //     else if (msg->type == MsgType::Kill) {
            //         delete decoded_strings;
            //         delete[] buffer;
            //         return;
            //     }
            //     delete msg;
            // }
            
            // delete decoded_strings;
            delete[] buffer;
        }
    }
};