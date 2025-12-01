#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "protocol.h"
#include "network.h"
#include "time_service.h"

typedef struct
{
		timezone_t timezones[MAX_CLIENTS];
		int timezone_count;
		pthread_mutex_t data_mutex;
} server_state_t;

server_state_t server_state = {
		.timezone_count = 0,
		.data_mutex = PTHREAD_MUTEX_INITIALIZER
};

extern volatile sig_atomic_t shutdown_requested;

void send_error(int client_socket, int code, const char *message)
{
		error_resp_t error;
		error.code = code;
		strncpy(error.message, message, sizeof(error.message) - 1);

		message_t msg;
		memset(&msg, 0, sizeof(msg));
		msg.type = MSG_ERROR;
		memcpy(msg.data, &error, sizeof(error));
		msg.length = sizeof(msg);

		send_message(client_socket, &msg);
}

void handle_get_time(int client_socket)
{
		char time_str[64];
		get_current_time_string(time_str, sizeof(time_str));

		message_t msg;
		memset(&msg, 0, sizeof(msg));
		msg.type = MSG_GET_TIME;
		strncpy(msg.data, time_str, sizeof(msg.data) - 1);
		msg.length = sizeof(msg);

		send_message(client_socket, &msg);
		printf("Sent time to client: %s\n", time_str);
}

void handle_create_timezone(int client_socket, const char *username, const char *data)
{
		char name[MAX_TIMEZONE], location[MAX_TIMEZONE];
		int offset_h, offset_m;

		if(sscanf(data, "%s %d %d %s", name, &offset_h, &offset_m, location) != 4)
		{
				send_error(client_socket, 400, "Invalid format. Use: create <name> <offset_h> <offset_m> <location>");
				return;
		}

		pthread_mutex_lock(&server_state.data_mutex);
		int id = create_timezone(server_state.timezones, &server_state.timezone_count, name, offset_h, offset_m, location, username);
		pthread_mutex_unlock(&server_state.data_mutex);

		if(id > 0)
		{
				char response[64];
				snprintf(response, sizeof(response), "Timezone '%s' created with ID %d", name, id);

				message_t msg;
				memset(&msg, 0, sizeof(msg));
				msg.type = MSG_CREATE_TIMEZONE;
				strncpy(msg.data, response, sizeof(msg.data) - 1);
				msg.length = sizeof(msg);

				send_message(client_socket, &msg);
				printf("Created timezone '%s' for user '%s'\n", name, username);
		}
		else
		{
				send_error(client_socket, 500, "Failed to create timezone - storage full");
		}
}

void handle_list_timezone(int client_socket, const char *username)
{
		char result[512];

		pthread_mutex_lock(&server_state.data_mutex);
		list_user_timezones(server_state.timezones, server_state.timezone_count, username, result, sizeof(result));
		pthread_mutex_unlock(&server_state.data_mutex);

		message_t msg;
		memset(&msg, 0, sizeof(msg));
		msg.type = MSG_LIST_TIMEZONES;
		strncpy(msg.data, result, sizeof(msg.data) - 1);
		msg.length = sizeof(msg);

		send_message(client_socket, &msg);
		printf("Sent timezone list to user '%s'\n", username);
}

void *handle_client(void *arg)
{
		int client_socket = *(int *)arg;
		free(arg);

		char username[MAX_USERNAME] = "anonymous";
		printf("Client connected on socket %d\n", client_socket);

		message_t msg;
		while(!shutdown_requested && receive_message(client_socket, &msg) == 0)
		{
				printf("Recieved message type: %d\n", msg.type);

				switch(msg.type)
				{
				case MSG_LOGIN:
						login_req_t *login = (login_req_t *)msg.data;
						strncpy(username, login->username, sizeof(username) - 1);
						printf("Client authenticated as: %s\n", username);

						message_t response;
						init_message(&response, MSG_LOGIN, "Login successful");
						send_message(client_socket, &response);
						break;

				case MSG_GET_TIME:
						handle_get_time(client_socket);
						break;

				case MSG_CREATE_TIMEZONE:
						handle_create_timezone(client_socket, username, msg.data);
						break;

				case MSG_LIST_TIMEZONES:
						handle_list_timezone(client_socket, username);
						break;

				case MSG_PING:
						message_t ping;
						init_message(&ping, MSG_PING, "PING");
						send_message(client_socket, &ping);
						printf("Sent ping to client\n");
						break;

				default:
						send_error(client_socket, 400, "Unknown command");
						break;
				}
		}

		close(client_socket);
		printf("Client %s disconnected\n", username);
		return NULL;
}

void run_server(int port) {
		int server_socket = create_server_socket(port);
		if (server_socket < 0) {
				fprintf(stderr, "Failed to create server socket\n");
				return;
		}

		printf("Server is running. Waiting for connections...\n");

		while (!shutdown_requested) {
				int client_socket = accept_client(server_socket);
				if (client_socket < 0) {
						if (!shutdown_requested) {
								perror("accept failed");
						}
						continue;
				}

				pthread_t thread_id;
				int *client_sock = malloc(sizeof(int));
				*client_sock = client_socket;

				if (pthread_create(&thread_id, NULL, handle_client, client_sock) != 0) {
						perror("pthread_create");
						close(client_socket);
						free(client_sock);
				} else {
						pthread_detach(thread_id);
				}
		}

		printf("Shutting down server...\n");
		close_server_socket(server_socket);
}
