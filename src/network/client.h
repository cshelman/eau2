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
        vector<Message*>* msgs = parse_msg(MsgType::Put, key, contents, node->id);
        for (int i = 0; i < msgs->size(); i++) {
            string serialized_msg = serialize_message(msgs->at(i));
            char* encoded_msg = encode(serialized_msg);
            send(server_sock, encoded_msg, strlen(encoded_msg) + 1, 0);
        }
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
                    char* decoded = new char[i - start - 2];
                    memcpy(decoded, s + start, i - start - 2);
                    string* str = new string(decoded);
                    decoded_strings->push_back(*str);
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

            Message* msg = deserialize_message(buffer);
            if (msg->type == MsgType::Act) {
                node->apply(rower, msg->key);           
                send_message(msg->key, rower->serialize());
            }
            else if (msg->type == MsgType::Get) {
                char* df = node->get(msg->key);
                send_message(msg->key, df);
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
                delete[] buffer;
                return;
            }

            delete msg;
            delete[] buffer;
        }
    }

    vector<Message*>* parse_msg(MsgType type, Key* key, char* s, size_t sender) {
        vector<Message*>* messages = new vector<Message*>();
        string* str = new string(s);
        int chunk_size = 500;
        if (str->size() < chunk_size) {
            chunk_size = str->size() - 1;
        }
        int start = 0;
        int end = chunk_size;

        while (true) {
            Message* msg = new Message(type, key, (char*)str->substr(start, end - start).c_str(), node->id);
            messages->push_back(msg);

            if (end == str->size()) {
                break;
            }
            start = end;
            end += chunk_size;
            if (end > str->size()) {
                end = str->size();
            }
        }
        Message* end_msg = new Message(type, key, (char*)"END", node->id);
        messages->push_back(end_msg);
        return messages;
    }
};