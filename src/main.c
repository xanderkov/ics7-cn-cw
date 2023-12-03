//
// Created by Александр Ковель on 26.11.2023.
//

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "server/socket_poll.h"
#include "server/server.h"
#include "logger/logger.h"

#define PORT 9990

server_t *server = NULL;


void sig_handler(int signum) {
	LOG_INFO("Received signal %d\n", signum);
	free_http_server_t(server);
	exit(0);
}

int main(int argc, char** argv)
{
	char *host = "0.0.0.0";
	int port = PORT, thread_num = THREAD_NUM;

	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGKILL, sig_handler);
	signal(SIGHUP, sig_handler);

	server = new_http_server(host, port, thread_num);

	if (run_http_server_t(server) < 0)
	{
		free_http_server_t(server);
	}



	return 0;
}
