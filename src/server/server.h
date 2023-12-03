//
// Created by Александр Ковель on 03.12.2023.
//

#ifndef SERVER_THREAD_SERVER_SERVER_H_
#define SERVER_THREAD_SERVER_SERVER_H_

#include "thread_pool.h"
#include <poll.h>

#define HOST_SIZE 16
#define THREAD_NUM 11
#define PATH_NUM 256
#define HEADER_LEN 128

typedef struct server_t {
	char host[HOST_SIZE];
	int port;

	struct pollfd *clients;
	long cl_num;

	int listen_sock;
	tpool_t *pool;

	char *wd;
} server_t;

server_t *new_http_server(char *host, int port, int thread_num);

int run_http_server_t(server_t *server);

void free_http_server_t(server_t *server);

#endif //SERVER_THREAD_SERVER_SERVER_H_
