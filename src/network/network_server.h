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

//communicates with clients
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
        delete master_queue;
        delete client_ips;
        delete client_sockets;
        delete[] ts;
    }

    //adds number of bytes in the given string as 4 digit number 
    //padded with 0s to the beginning of the string
    string* encode(string s) {
        stringstream ss;
        ss << setw(4) << setfill('0') << (s.size() + 1);
        string* result = new string(ss.str());
        result->append(s);

        return result;
    }

    //starts listening on the given ip/port
    void startup() {
        int yes = 1;
        assert((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) > 0);
        assert(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &yes, sizeof(yes)) >= 0);
        assert(bind(sock_fd, (struct sockaddr*)&serv, servlen) >= 0);
        assert(listen(sock_fd, 10) >= 0);
    }

    //encodes and then sends message to given client
    void send_msg(int node, Message* msg) {
        string* s = serialize_message(msg);
        string* encoded = encode(*s);
        int bytes = send(atoi(client_sockets->at(node)), (char*)encoded->c_str(), encoded->size() + 1, 0);
        delete encoded;
        delete s;
    }

    //listens for incoming messages from each client
    //runs in a different thread for each client connected to network
    void listening(char* client_sock) {
        while (true) {
            char* size = new char[5];
            size[4] = '\0';

            recv(atoi(client_sock), size, 4, 0);
            int bytes_to_read = atoi(size);
            delete[] size;

            int bytes_read = 0;
            if (bytes_to_read > 0) {
                char* buffer = new char[bytes_to_read];
                memset(buffer, '\0', bytes_to_read);

                while (bytes_read < bytes_to_read) {
                    bytes_read += recv(atoi(client_sock), buffer, bytes_to_read - bytes_read, 0);
                }
                
                Message* msg = deserialize_message((char*)buffer);
                master_queue->push(msg);
                delete[] buffer;
            }
            else {
                break;
            }
        }
    }

    //queue holds messages from clients in received order
    Message* recv_master() {
        return master_queue->pop();
    }

    //waits for the given number of clients to join the network
    void registration() {
        printf("Server waiting for %d clients to connect\n", num_nodes);
        while (client_ips->size() < num_nodes) {
            string* buffer = new string();
            int client_sock;
            char temp_buffer[NET_BUF_SIZE];
            assert((client_sock = accept(sock_fd, (struct sockaddr*)&serv,(socklen_t*)&servlen)) >= 0);
            
            int status;
            assert((status = recv(client_sock, temp_buffer, NET_BUF_SIZE, 0)) >= 0);
            buffer->append(temp_buffer);

            client_ips->push_back((char*)buffer->c_str());
            delete buffer;

            int index = client_sockets->size();
            char* sock = new char[16];
            strcpy(sock, (char*)to_string(client_sock).c_str());
            client_sockets->push_back(sock);

            char* assigned_node_num = (char*)to_string(index).c_str();
            send(atoi(client_sockets->at(index)), assigned_node_num, strlen(assigned_node_num) + 1, 0);
            ts[index] = new thread(&NetworkServer::listening, this, sock);
            printf("\tClient %ld connected\n", client_ips->size());
        }
        printf("\n");
    }

    //kills all threads listening to the clients
    void shutdown() {
        alive = false;
        for (int i = 0; i < client_sockets->size(); i++) {
            ts[i]->join();
            close(atoi(client_sockets->at(i)));
            delete ts[i];
            delete[] client_sockets->at(i);
        }
    }
};