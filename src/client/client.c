#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ncurses.h>
#include "protocol.h"
#include "network.h"
#include "ui.h"

typedef struct {
    int socket;
    int running;
    char username[MAX_USERNAME];
} client_state_t;

void *receive_handler(void *arg) {
    client_state_t *state = (client_state_t *)arg;
    message_t msg;

    while (state->running) {
        if (receive_message(state->socket, &msg) == 0) {
            switch (msg.type) {
                case MSG_GET_TIME:
                case MSG_CREATE_TIMEZONE:
                case MSG_LIST_TIMEZONES:
                case MSG_LOGIN:
                case MSG_ERROR:
                    display_message(msg.data);
                    break;

                case MSG_PING:
                    display_message("Server responded with PING");
                    break;

                default:
                    display_message("Unknown message type from server");
                    break;
            }
        } else {
            display_message("Connection lost to server");
            state->running = 0;
            break;
        }
    }

    return NULL;
}

void send_login(int socket, const char *username) {
    login_req_t login;
    strncpy(login.username, username, sizeof(login.username) - 1);

    message_t msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = MSG_LOGIN;
    memcpy(msg.data, &login, sizeof(login));
    msg.length = sizeof(msg);

    send_message(socket, &msg);
}

void send_ping(int socket) {
    message_t msg;
    init_message(&msg, MSG_PING, "PING");
    send_message(socket, &msg);
}

void send_get_time(int socket) {
    message_t msg;
    init_message(&msg, MSG_GET_TIME, "");
    send_message(socket, &msg);
}

void send_create_timezone(int socket, const char *name, int offset_h, int offset_m, const char *location) {
    char request[256];
    snprintf(request, sizeof(request), "%s %d %d %s", name, offset_h, offset_m, location);

    message_t msg;
    init_message(&msg, MSG_CREATE_TIMEZONE, request);
    send_message(socket, &msg);
}

void send_list_timezones(int socket) {
    message_t msg;
    init_message(&msg, MSG_LIST_TIMEZONES, "");
    send_message(socket, &msg);
}

void run_client(const char *server_host, const char *username) {
    int client_socket = connect_to_server(server_host, SERVER_PORT);
    if (client_socket < 0) {
        fprintf(stderr, "Failed to connect to server\n");
        return;
    }

    init_ui();

    client_state_t state = {
        .socket = client_socket,
        .running = 1,
        .username = ""
    };
    strncpy(state.username, username, sizeof(state.username) - 1);

    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_handler, &state);

    send_login(client_socket, username);
    display_message("Login request sent...");

    char input[256];
    while (state.running) {
        char *user_input = get_user_input("Enter command: ");
        if (user_input == NULL || strlen(user_input) == 0) {
            continue;
        }

        strncpy(input, user_input, sizeof(input) - 1);
        if (strcmp(input, "quit") == 0) {
            state.running = 0;
            display_message("Disconnecting...");
        } else if (strcmp(input, "help") == 0) {
            show_help();
        } else if (strcmp(input, "time") == 0) {
            send_get_time(client_socket);
        } else if (strcmp(input, "ping") == 0) {
            send_ping(client_socket);
        } else if (strcmp(input, "list") == 0) {
            send_list_timezones(client_socket);
        } else if (strncmp(input, "create", 6) == 0) {
            char name[32], location[32];
            int offset_h, offset_m;

            if (sscanf(input, "create %s %d %d %s", name, &offset_h, &offset_m, location) == 4) {
                send_create_timezone(client_socket, name, offset_h, offset_m, location);
            } else {
                display_message("Usage: create <name> <offset_h> <offset_m> <location>");
            }
        } else {
            display_message("Unknown command. Type 'help' for available commands.");
        }
    }

    state.running = 0;
    pthread_join(recv_thread, NULL);
    close_client_socket(client_socket);
    cleanup_ui();

    printf("Client shutdown complete.\n");
}
