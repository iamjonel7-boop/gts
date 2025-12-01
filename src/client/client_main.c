#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"

extern void run_client(const char *server_host, const char *username);

int main(int argc, char *argv[])
{
		if(argc < 3)
		{
				fprintf(stderr, "Usage: %s <server> <username>\n", argv[0]);
				return 1;
		}

		run_client(argv[1], argv[2]);
		return 0;
}
