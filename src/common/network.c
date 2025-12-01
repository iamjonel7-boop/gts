#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "network.h"
#include "protocol.h"

int create_server_socket(int port)
{
		int server_socket;
		struct sockaddr_in sa;

		if((server_socket = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		{
				perror("socket");
				return -1;
		}

		int opt = 1;
		setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

		memset(&sa, 0, sizeof(sa));
		sa.sin_family = AF_INET;
		sa.sin_port = htons(port);
		sa.sin_addr.s_addr = htonl(INADDR_ANY);

		if(bind(server_socket, (struct sockaddr*)&sa, sizeof(sa)) < 0)
		{
				perror("bind");
				close(server_socket);
				return -1;
		}

		if(listen(server_socket, 10) < 0)
		{
				perror("listen");
				close(server_socket);
				return -1;
		}

		return server_socket;
}

int accept_client(int server_socket)
{
		struct sockaddr_in client_addr;
		socklen_t addr_len = sizeof(client_addr);
		int client_socket;

		client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
		if(client_socket < 0)
		{
				perror("accept");
				return -1;
		}

		printf("Client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		return client_socket;
}

void close_server_socket(int server_socket)
{
		close(server_socket);
}

int connect_to_server(const char *hostname, int port)
{
		int client_socket;
		struct sockaddr_in sa;
		struct hostent *server;

		printf("Connecting to %s:%d...\n", hostname, port);

		if((client_socket = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		{
				perror("socket");
				return 1;
		}

		server = gethostbyname(hostname);
		if(server == NULL)
		{
				fprintf(stderr, "Error: no such host '%s'\n", hostname);
				close(client_socket);
				return -1;
		}

		memset(&sa, 0, sizeof(sa));
		sa.sin_family = AF_INET;
		sa.sin_port = htons(port);
		memcpy(&sa.sin_addr.s_addr, server->h_addr, server->h_length);

		if(connect(client_socket, (struct sockaddr*)&sa, sizeof(sa)) < 0)
		{
				perror("connect");
				close(client_socket);
				return -1;
		}

		printf("Connected to server succesfully\n");
		return client_socket;
}

void close_client_socket(int client_socket)
{
		close(client_socket);
}

int send_message(int socket, const message_t *msg)
{
		ssize_t bytes_sent = write(socket, msg, sizeof(message_t));
		if(bytes_sent < 0)
		{
				perror("send_message");
				return -1;
		}
		return 0;
}

int receive_message(int socket, message_t *msg)
{
		ssize_t bytes_read = read(socket, msg, sizeof(message_t));
		if(bytes_read <= 0)
		{
				if(bytes_read < 0) perror("recieve_message");
				return -1;
		}
		return 0;
}

void init_message(message_t *msg, uint32_t type, const char *data)
{
		memset(msg, 0, sizeof(message_t));
		msg->type = type;
		msg->seq = 1;
		if(data) strncpy(msg->data, data, MAX_MESSAGE - 1);
		msg->length = sizeof(message_t);
}
