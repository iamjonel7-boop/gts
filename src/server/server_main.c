#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "protocol.h"
#include "network.h"

void run_server(int port);

volatile sig_atomic_t shutdown_requested = 0;
int server_socket_fd = -1;

void handle_signal(int sig)
{
		(void)sig;
		shutdown_requested = 1;
		if (server_socket_fd != -1)
		{
				close(server_socket_fd);
				server_socket_fd = -1;
		}
}

void setup_signals(void) {
		struct sigaction sa;

		sa.sa_handler = handle_signal;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sigaction(SIGINT, &sa, NULL);

		sigaction(SIGTERM, &sa, NULL);

		sa.sa_handler = SIG_IGN;
		sigaction(SIGPIPE, &sa, NULL);
}

int main()
{
		setup_signals();

		printf("Global Time Service Server \n");
		printf("Starting on port %d...\n", SERVER_PORT);
		printf("Press Ctrl+C to shutdown\n\n");

		run_server(SERVER_PORT);

		printf("Server shutdown complete.\n");
    	return 0;
}
