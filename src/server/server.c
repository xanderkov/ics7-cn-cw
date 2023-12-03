//
// Created by Александр Ковель on 03.12.2023.
//

#include "server.h"
#include "logger.h"
#include "socket_poll.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>


server_t *new_http_server(char host[HOST_SIZE], int port, int thread_num)
{
	server_t *server = calloc(1, sizeof(server_t));
	if (server == NULL)
		return NULL;
	strcpy(server->host, host);
	server->port = port;

	server->cl_num = sysconf(_SC_OPEN_MAX);
	if (server->cl_num < 0) {
		LOG_ERROR("Failed to alloc clients fds");
		free(server);
		return NULL;
	}

	for (ssize_t i = 0; i < server->cl_num; i++) {
		server->clients[i].fd = -1;
	}

	server->pool = tpool_create(thread_num);
	if (server->pool == NULL)
	{
		LOG_ERROR("Failed to create pool");
		free(server->clients);
		free(server);
		return NULL;
	}

	server->wd = calloc(PATH_NUM, sizeof(char));
	if (server->wd == NULL)
	{
		LOG_ERROR("Failed to alloc wd");
		tpool_destroy(server->pool);
		free(server->clients);
		free(server);
		return NULL;
	}

	if (getcwd(server->wd, PATH_NUM) == NULL)
	{
		LOG_ERROR("Failed to get wd");
		free(server->wd);
		tpool_destroy(server->pool);
		free(server->clients);
		free(server);
		return NULL;
	}

	LOG_INFO("Work dir: %s", server->wd);
	LOG_INFO("Server created");

	return server;
}


int run_http_server_t(server_t *server)
{
	server->listen_sock = creat_socket(server->port, server->host);
	int client_socket = wait_client(server->listen_sock);

}

void free_http_server_t(server_t *server) {
	server->clients[0].revents = -1;
	close(server->listen_sock);

	tpool_wait(server->pool);
	tpool_destroy(server->pool);

	LOG_INFO("server stopped");

	free(server->wd);
	free(server->clients);
	free(server);
}