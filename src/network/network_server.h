#pragma once

#include "../serializer/serial.h"
#include "message.h"
#include "message_queue.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <assert.h>
#include <poll.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>
#include <vector>
#include <sstream>
#include <iomanip>

#include <errno.h>

#define NET_BUF_SIZE 8192

using namespace std;

class NetworkServer {
public:
    char* ip;
    char* port;
    vector<char*>* client_ips;
    vector<char*>* client_sockets;
    struct sockaddr_in serv;
    int servlen;
    int sock_fd;
    int num_nodes;
    MessageQueue* master_queue;
    thread** ts;
    bool alive;

    NetworkServer(char* address, int num_nodes_) {
        num_nodes = num_nodes_;
        ip = strtok(address, ":");
        port = strtok(NULL, ":");
        client_ips = new vector<char*>();
        client_sockets = new vector<char*>();

        serv.sin_family = AF_INET;
        serv.sin_port = htons(atoi(port));
        assert(inet_pton(AF_INET, ip, &serv.sin_addr) > 0);
        servlen = sizeof(serv);
        master_queue = new MessageQueue();
        ts = new thread*[num_nodes_];
        alive = true;
    }

    ~NetworkServer() {
        delete client_ips;
        delete client_sockets;
        delete ts;
    }

    // backticks are not a valid character to send
    char* encode(string s) {
        // string* encoded_s = new string("```");
        // encoded_s->append(s);
        // encoded_s->append("```");
        // return (char*)encoded_s->c_str();
        
        stringstream ss;
        ss << setw(4) << setfill('0') << (s.size() + 1);
        string* result = new string(ss.str());
        result->append(s);
        // printf("encoded string: %s\n", (char*)result->c_str());

        return (char*)result->c_str();
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

    void startup() {
        int yes = 1;
        assert((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) > 0);
        assert(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &yes, sizeof(yes)) >= 0);
        assert(bind(sock_fd, (struct sockaddr*)&serv, servlen) >= 0);
        assert(listen(sock_fd, 10) >= 0);
    }

    void send_msg(int node, Message* msg) {
        Message* temp = msg->copy();
        string s = serialize_message(msg);
        // printf("message: %s\n", (char*)s.c_str());
        char* encoded = encode(s);
        // printf("net_server encoded message: %s\n", encoded);
        int bytes = send(atoi(client_sockets->at(node)), encoded, strlen(encoded) + 1, 0);
    }

    void listening(char* client_sock) {
        while(true) {
            char* size = new char[4];

            recv(atoi(client_sock), size, 4, 0);
            int bytes_read = 0;
            int bytes_to_read = atoi(size);

            if (bytes_to_read > 0) {
                char* buffer = new char[bytes_to_read];
                memset(buffer, '\0', bytes_to_read);

                while (bytes_read < bytes_to_read) {
                    bytes_read += recv(atoi(client_sock), buffer, bytes_to_read - bytes_read, 0);
                }
                
                Message* msg = deserialize_message((char*)buffer);
                master_queue->push(msg);
            }
            else {
                break;
            }
        }
    }

    Message* recv_master() {
        return master_queue->pop();
    }

    void registration() {
        while (client_ips->size() < num_nodes) {
            string* buffer = new string();
            int client_sock;
            char temp_buffer[NET_BUF_SIZE];
            assert((client_sock = accept(sock_fd, (struct sockaddr*)&serv,(socklen_t*)&servlen)) >= 0);
            
            int status;
            assert((status = recv(client_sock, temp_buffer, NET_BUF_SIZE, 0)) >= 0);
            buffer->append(temp_buffer);

            client_ips->push_back((char*)buffer->c_str());

            int index = client_sockets->size();
            char* sock = new char[16];
            strcpy(sock, (char*)to_string(client_sock).c_str());
            client_sockets->push_back(sock);

            char* assigned_node_num = (char*)to_string(index).c_str();
            send(atoi(client_sockets->at(index)), assigned_node_num, strlen(assigned_node_num) + 1, 0);
            ts[index] = new thread(&NetworkServer::listening, this, sock);
        }
    }

    void shutdown() {
        alive = false;
        for (int i = 0; i < client_sockets->size(); i++) {
            ts[i]->join();
            close(atoi(client_sockets->at(i)));
            delete[] client_sockets->at(i);
        }
    }
};