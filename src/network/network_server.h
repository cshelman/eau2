// #pragma once

// #include <string.h>
// #include <stdio.h>
// #include <unistd.h>
// #include <netdb.h>
// #include <stdlib.h>
// #include <assert.h>
// #include <poll.h>
// #include <sys/types.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <sys/socket.h>
// #include <thread>
// #include <vector>

// #include <errno.h>

// #define BUFFER_SIZE 4096

// using namespace std;

// class NetworkServer {
// public:
//     char* ip;
//     char* port;
//     vector<char*>* client_ips;
//     vector<char*>* client_sockets;
//     struct sockaddr_in serv;
//     int servlen;
//     int sock_fd;

//     NetworkServer(char* address) {
//         ip = strtok(address, ":");
//         port = strtok(NULL, ":");
//         client_ips = new vector<char*>();
//         client_sockets = new vector<char*>();

//         serv.sin_family = AF_INET;
//         serv.sin_port = htons(atoi(port));
//         assert(inet_pton(AF_INET, ip, &serv.sin_addr) > 0);
//         servlen = sizeof(serv);
//     }

//     ~NetworkServer() {
//         delete client_ips;
//         delete client_sockets;
//     }

//     // backticks are not a valid character to send
//     char* encode(char* s) {
//         char* encoded_s = new char[strlen(s) + 3];
//         strcpy(encoded_s, "`");
//         encoded_s = strcat(encoded_s, s);
//         encoded_s = strcat(encoded_s, "`");
//         return encoded_s;
//     }

//     void startup() {
//         int yes = 1;
//         assert((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) > 0);
//         assert(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &yes, sizeof(yes)) >= 0);
//         assert(bind(sock_fd, (struct sockaddr*)&serv, servlen) >= 0);
//         assert(listen(sock_fd, 10) >= 0);
//     }

//     void distribute_ips() {
//         for (int i = 0; i < client_sockets->size(); i++) {
//             for (int j = 0; j < client_ips->size(); j++) {
//                 char* encoded = encode(client_ips->at(j));
//                 int sent = send(atoi(client_sockets->at(i)), encoded, strlen(encoded) + 1, 0);
//             }
//         }
//     }

//     void send_msg(int node, Message msg) {
//         char* encoded = encode(client_ips->at(node));
//         send(atoi(client_sockets->at(node)), encoded, strlen(encoded) + 1, 0);
//     }

//     void registration() {
//         while (true) {
//             string* buffer = new string();
//             int client_sock;
//             char temp_buffer[BUFFER_SIZE];
//             assert((client_sock = accept(sock_fd, (struct sockaddr*)&serv,(socklen_t*)&servlen)) >= 0);
            
//             int status;
//             assert((status = recv(client_sock, temp_buffer, BUFFER_SIZE, 0)) >= 0);
//             buffer->append(temp_buffer);
//             printf("client %s connected\n", buffer->c_str());

//             client_ips->push_back((char*)buffer->c_str());

//             char num[16];
//             sprintf(num, "%d", client_sock);
//             client_sockets->push_back(num);

//             distribute_ips();
//         }
//     }
// };