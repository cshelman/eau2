#pragma once

#include "node.h"
#include "../rowers/word_count.h"
#include "../rowers/find_projects.h"
#include "../rowers/find_users.h"

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
    int rower_type;

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

        rower = nullptr;
    }

    ~Client() {
        delete_rower();
        delete node;
        delete[] address;
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
            string* serialized_msg = serialize_message(msgs->at(i));
            string* encoded_msg = encode(serialized_msg);
            send(server_sock, (char*)encoded_msg->c_str(), encoded_msg->size() + 1, 0);
            delete msgs->at(i);
            delete serialized_msg;
            delete encoded_msg;
        }
        delete msgs;
    }

    //adds number of bytes in the given string as 4 digit number 
    //padded with 0s to the beginning of the string
    string* encode(string* s) {
        stringstream ss;
        ss << setw(4) << setfill('0') << (s->size() + 1);
        string* result = new string(ss.str());
        result->append(*s);

        return result;
    }

    void delete_rower() {
        if (rower_type == 0) {
            FindProjectsRower* cr = dynamic_cast<FindProjectsRower*>(rower);
            delete cr;
        }
        else if (rower_type == 1) {
            FindUsersRower* cr = dynamic_cast<FindUsersRower*>(rower);
            delete cr;
        }
        else if (rower_type == 2) {
            WordCountRower* cr = dynamic_cast<WordCountRower*>(rower);
            delete cr;
        }
        else {
            delete rower;
        }
        rower = nullptr;
    }

    void set_up_rower(char* r) {
        if (rower != nullptr) {
            delete_rower();
        }        
        //not sure how to do this better, open to suggestions
        //one idea: pass in a list of functions to the client which return 
        //          the correct instance of rower based on the index
        if (r[0] == '0') {
            rower = new FindProjectsRower(r);
            rower_type = 0;
        } 
        else if (r[0] == '1') {
            rower = new FindUsersRower(r);
            rower_type = 1;
        }
        else if (r[0] == '2') {
            rower = new WordCountRower(r);
            rower_type = 2;
        }
    }

    void be_client() {
        //gets the node id from the server (server assigns the clients node ids)
        char* node_id = new char[8];
        memset(node_id, '\0', 8);
        read(server_sock, node_id, 8);
        node = new Node(atoi(node_id));
        delete[] node_id;
       
        //used to construct messages
        string* put_msg = new string("");
        string* rower_msg = new string("");

        //start listening to messages from the server
        while (true) {
            //size is the number of bytes to read (every message is encoded with this in encode)
            char* size = new char[5];
            size[4] = '\0';
            
            recv(server_sock, size, 4, 0);
            int bytes_read = 0;
            int bytes_to_read = atoi(size);
            char* buffer = new char[bytes_to_read];
            memset(buffer, '\0', bytes_to_read);

            //reads in given number of bytes
            while (bytes_read < bytes_to_read) {
                bytes_read += recv(server_sock, buffer, bytes_to_read - bytes_read, 0);
            }

            delete[] size;

            Message* msg = deserialize_message(buffer);

            //checks message type and does the proper action

            if (msg->type == MsgType::SetRower) { // updates the rower for this client
                //waits for the whole message, "END" means end of message
                if (strcmp(msg->contents, "END") == 0) {
                    set_up_rower((char*)rower_msg->c_str());
                    delete rower_msg;
                    rower_msg = new string();
                }
                else {
                    rower_msg->append(msg->contents);
                }
            }
            else if (msg->type == MsgType::Act) { // runs this client's rower on the df corresponding to the given key
                printf("starting to apply rower\n");
                node->apply(rower, msg->key);
                printf("done applying rower\n");
                printf("start serializing rower\n");
                string* serialized_rower = rower->serialize();
                printf("done serializing\n");
                printf("starting send\n");
                send_message(msg->key, (char*)serialized_rower->c_str());
                printf("done send\n");
                delete serialized_rower;
            }
            else if (msg->type == MsgType::Get) { // gets the df corresponding to the given key
                char* df = node->get(msg->key);
                send_message(msg->key, df);
                delete[] df;
            }
            else if (msg->type == MsgType::Put) { // puts the df in our data store
                if (strcmp(msg->contents, "END") == 0) {
                    node->put(msg->key, (char*)put_msg->c_str());
                    delete put_msg;
                    put_msg = new string();
                }
                else {
                    put_msg->append(msg->contents);
                }
            }
            else if (msg->type == MsgType::Kill) { // kills this client, initiated by network shutdown
                delete msg;
                delete[] buffer;
                delete rower_msg;
                delete put_msg;
                return;
            }

            delete msg;
            delete[] buffer;
        }
    }

    //splits up large messages into list of smaller messages TODO: move to util file
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
        delete str;
        return messages;
    }
};