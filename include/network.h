#ifndef NETWORK_H
#define NETWORK_H

#include "protocol.h"

int create_server_socket(int port);
int accept_client(int server_socket);
void close_server_socket(int server_socket);

int connect_to_server(const char *hostname, int port);
void close_client_socket(int client_socket);

int send_message(int socket, const message_t *msg);
int receive_message(int socket, message_t *msg);

void init_message(message_t *msg, uint32_t type, const char *data);

#endif
