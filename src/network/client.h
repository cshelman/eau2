// #pragma once

// #include <string.h>
// #include <stdio.h>
// #include <unistd.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netdb.h>
// #include <stdlib.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <assert.h>
// #include <poll.h>
// #include <thread>
// #include <vector>
// #include <algorithm>

// #include <errno.h>

// using namespace std;

// #define BUFFER_SIZE 4096

// class Client {
// public:
//     char* ip;
//     char* port;
//     char* address;
//     const char* server_ip;
//     const char* server_port;
//     vector<char*>* neighbor_ips;
//     vector<char*>* neighbor_sockets;
//     thread** neighbor_threads;
//     struct sockaddr_in client;
//     int clientlen;
//     int server_sock;

//     Client(char* address_in, char* server_address) {
//         address = new char[strlen(address_in) + 1];
//         strcpy(address, address_in);

//         ip = strtok(address_in, ":");
//         port = strtok(NULL, ":");
//         neighbor_ips = new vector<char*>();
//         neighbor_sockets = new vector<char*>();

//         client.sin_family = AF_INET;
//         client.sin_port = htons(atoi(port));
//         assert(inet_pton(AF_INET, ip, &client.sin_addr) > 0);
//         clientlen = sizeof(client);

//         server_ip = strtok(server_address, ":");
//         server_port = strtok(NULL, ":");

//         neighbor_threads = new thread*[512];
//     }

//     ~Client() {
//         delete neighbor_ips;
//         delete neighbor_sockets;
//     }

//     // create connection with server
//     void register_ip() {
//         struct sockaddr_in to_serv;
//         assert((server_sock = socket(AF_INET, SOCK_STREAM, 0)) >= 0);
//         to_serv.sin_family = AF_INET;
//         to_serv.sin_port = htons(atoi(server_port));
//         assert(inet_pton(AF_INET, server_ip, &to_serv.sin_addr) > 0);
//         assert(connect(server_sock, (struct sockaddr*)&to_serv, sizeof(to_serv)) >= 0);

//         int sent_bytes = 0;
//         while (sent_bytes < sizeof(address) / sizeof(char)) {
//             int ret;
//             assert((ret = send(server_sock, address, strlen(address) + 1, 0)) >= 0);
//             sent_bytes += ret;
//         }
//     }

//     void send_message(char* buffer) {
//         for (int i = 0; i < neighbor_sockets->size(); i++) {
//             printf("sending %s to %s(%s)\n", buffer, neighbor_ips->at(i), neighbor_sockets->at(i));
//             send(atoi(neighbor_sockets->at(i)), buffer, strlen(buffer) + 1, 0);
//         }
//     }

//     vector<char*>* decode(char* s, int bytes_read) {
//         vector<char*>* decoded_strings = new vector<char*>();
//         int start = 0;
//         bool in_msg = false;

//         for (int i = 0; i < bytes_read; i++) {
//             if (s[i] == '`') {
//                 if (in_msg) {
//                     char* decoded = new char[i - start + 1];
//                     memcpy(decoded, s + start, i - start);
//                     decoded_strings->push_back(decoded);
//                     delete[] decoded;
//                 }
//                 else {
//                     start = i + 1;
//                 }
//                 in_msg = !in_msg;
//             }
//         }
//         return decoded_strings;
//     }

//     void create_neighbor_connection(char* neighbor_port, char* neighbor_ip) {
//         // creating input socket
//         struct sockaddr_in neigh;

//         neigh.sin_family = AF_INET;
//         neigh.sin_port = htons(atoi(neighbor_port));
//         assert(inet_pton(AF_INET, ip, &neigh.sin_addr) > 0);
//         int neighlen = sizeof(neigh);

//         int yes = 1;
//         int listen_sock;
//         assert((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) > 0);
//         assert(setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &yes, sizeof(yes)) >= 0);
//         assert(bind(listen_sock, (struct sockaddr*)&neigh, neighlen) >= 0);
//         assert(listen(listen_sock, 10) >= 0);

//         int read_sock;
//         assert((read_sock = accept(listen_sock, (struct sockaddr*)&neigh,(socklen_t*)&neighlen)) >= 0);

        
//         while (true) {

//             char* buffer = new char[BUFFER_SIZE];
//             memset(buffer, '\0', BUFFER_SIZE);
//             int bytes_read = read(read_sock, buffer, BUFFER_SIZE);
//             vector<char*>* decoded_strings = decode(buffer, bytes_read);

//             for (int i = 0; i < decoded_strings->size(); i++) {
//                 printf("[%s]: %s\n", neighbor_ip, decoded_strings->at(i));
//             }
//             delete decoded_strings;
//             delete[] buffer;
//         }
//     }

//     // backticks are not a valid character to send
//     char* encode(char* s) {
//         char* encoded_s = new char[strlen(s) + 3];
//         strcpy(encoded_s, "`");

//         char parsed_s[strlen(s)];
//         memcpy(parsed_s, &s[0], strlen(s) - 1);
//         parsed_s[strlen(s) - 1] = '\0';

//         encoded_s = strcat(encoded_s, parsed_s);
//         encoded_s = strcat(encoded_s, "`");
//         return encoded_s;
//     }

//     void communicate_with_neighbor() {
//         while(true) {
//             char* buffer = new char[BUFFER_SIZE];
//             fgets(buffer, BUFFER_SIZE, stdin);

//             for (int i = 0; i < neighbor_sockets->size(); i++) {
//                 char* encoded = encode(buffer);
//                 int sent = send(atoi(neighbor_sockets->at(i)), encoded, strlen(encoded) + 1, 0);
//             }
//         }
//     }

//     void be_client() {
//         thread* communicate_thread = new thread(&Client::communicate_with_neighbor, this);
       
//         while (true) {

//             char* buffer = new char[BUFFER_SIZE];
//             memset(buffer, '\0', BUFFER_SIZE);
//             int bytes_read = read(server_sock, buffer, BUFFER_SIZE);
//             vector<char*>* decoded_strings = decode(buffer, bytes_read);

//             for (int i = 0; i < decoded_strings->size(); i++) {

//                 if (count(neighbor_ips->begin(), neighbor_ips->end(), decoded_strings->at(i)) > 0 && strcmp(decoded_strings->at(i), address) != 0) {

//                     neighbor_ips->push_back(decoded_strings->at(i));
                    
//                     char* cur_ip = strtok(decoded_strings->at(i), ":");
//                     char* cur_port = strtok(NULL, ":");

//                     // thread to listen
//                     thread* neighbor_thread = new thread(&Client::create_neighbor_connection, this, cur_port, cur_ip);
                    
//                     struct sockaddr_in to_neigh;
//                     int neigh_sock;
//                     assert((neigh_sock = socket(AF_INET, SOCK_STREAM, 0)) >= 0);
//                     to_neigh.sin_family = AF_INET;
//                     to_neigh.sin_port = htons(atoi(port));
//                     assert(inet_pton(AF_INET, cur_ip, &to_neigh.sin_addr) > 0);

//                     while (connect(neigh_sock, (struct sockaddr*)&to_neigh, sizeof(to_neigh)) < 0) {}

                    
//                     char* temp = new char[8];
//                     sprintf(temp, "%d", neigh_sock);
//                     neighbor_sockets->push_back(temp);
//                     printf("%s:%s has joined the network\n", cur_ip, cur_port);
//                     delete[] temp;
//                 }
//             }
//             delete decoded_strings;
//             delete[] buffer;
//         }

//         for (int i = 0; i < neighbor_ips->size(); i++) {
//             neighbor_threads[i]->join();
//         }

//         for (int i = 0; i < neighbor_sockets->size(); i++) {
//             close(atoi(neighbor_sockets->at(i)));
//         }
//     }
// };