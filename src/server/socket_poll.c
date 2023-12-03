//
// Created by Александр Ковель on 26.11.2023.
//

#include "socket_poll.h"
#include "thread_pool.h"
#include "request.h"
#include "responses.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#define REQ_SIZE 2048
#define RESP_SIZE 1024

typedef struct worker_sock_t {
	char *wd;
	int clientfd;
} worker_sock_t;



int creat_socket(int port, char *host)
{
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(host);

	int bindResult = bind(server_socket, (struct sockaddr*)&addr, sizeof(addr));
	if (bindResult == -1)
	{
		LOG_ERROR("bindResult");
	}

	int listenResult = listen(server_socket, 5);
	if (listenResult == -1)
	{
		LOG_ERROR("listenResult");
	}
	LOG_INFO("SOCKET CREATED!!!");
	return server_socket;
}

int read_req(char *buff, int clientfd) {
	long byte_read = 0, msg_size = 0;

	byte_read = read(clientfd, buff, REQ_SIZE - 1);
	if (byte_read <= 0)
		return -1;
	msg_size = byte_read;
	buff[msg_size - 1] = '\0';

	return 0;
}

int write_response(int fd, const void *buf, size_t n) {
	int byte_write = write(fd, buf, n);
	if (byte_write < 0)
	{
		LOG_ERROR("write error");
	}
	return byte_write;
}

void send_err(int clientfd, const char *str) {
	LOG_INFO("%s", str);
	write_response(clientfd, str, strlen(str));
}

void worker(void* arg)
{
	worker_sock_t *worker_sock = arg;
	char *wd;
	int clientfd = worker_sock->clientfd;
	strcpy(wd, worker_sock->wd);
	request_t req;
	char *buff = calloc(REQ_SIZE, sizeof(char));
	if (buff == NULL) {
		LOG_ERROR("failed alloc req buf");
		return;
	}

	if (read_req(buff, clientfd) < 0) {
		send_err(clientfd, INT_SERVER_ERR_STR);
		close(clientfd);
		return;
	}

	if (parse_req(&req, buff) < 0) {
		send_err(clientfd, BAD_REQUEST_STR);
		close(clientfd);
		return;
	}
	if (req.method == BAD) {
		LOG_ERROR("unsupported http method");
		send_err(clientfd, M_NOT_ALLOWED_STR);
		close(clientfd);
		return;
	}

//	process_req(clientfd, &req, wd);
	LOG_INFO("OK");
	close(clientfd);
}

int accept_socket(int numfds, int maxcl, server_t *server)
{
	int client_sock = accept(server->listen_sock, NULL, 0);
	if (client_sock < 0)
	{
		return -1;
	}

	long i = 0;
	for (i = 1; i < server->cl_num; ++i) {
		if (server->clients[i].fd < 0) {
			server->clients[i].fd = client_sock;
			break;
		}
	}
	if (i == server->cl_num) {
		LOG_ERROR("too many connections");
		return -1;
	}
	server->clients[i].events = POLLIN;
	if (i > maxcl)
	{
		maxcl = i;
	}
	if (--numfds < 0)
	{
		maxcl = -1;
	}

	return maxcl;
}

void move_socket_after_close(struct pollfd *pollfds, int numfds, int fd_index)
{
	for (int i = fd_index; i < numfds; i++) {
		pollfds[i] = pollfds[i + 1];
	}
}

void analyze_client_text(struct pollfd *poll_fd, tpool_t *tm)
{
	tpool_add_work(tm, worker, poll_fd);
	tpool_wait(tm);
}

int read_from_socket(int fd_index, server_t *server)
{
	if (server->clients[fd_index].fd < 0)
		return -1;

	if (server->clients[fd_index].revents & (POLLIN | POLLERR)) {
		worker_sock_t worker_sock;
		worker_sock.clientfd = server->clients[fd_index].fd;
		strcpy(worker_sock.wd, server->wd);

		tpool_add_work(server->pool, worker, &worker_sock);
		server->clients[fd_index].fd = -1;
	}

	return 0;
}

void poll_sockets(int numfds, int maxcl, server_t *server)
{
	numfds = poll(server->clients, maxcl + 1, -1);

	if (numfds < 0) {
		LOG_ERROR("poll error");
		return;
	}

	if (server->clients[0].revents & POLLIN)
	{
		maxcl = accept_socket(numfds, maxcl, server);
		if (maxcl < 0)
			return;
	}

	for (int fd_index = 1; fd_index < maxcl + 1; fd_index++)
	{
		numfds = read_from_socket(fd_index, server);
	}
}

int wait_client(server_t *server)
{
	server->clients[0].fd = server->listen_sock;
	server->clients[0].events = POLLIN;
	int numfds = 0, maxcl = 0;


	while (1)
		poll_sockets(numfds, maxcl, server);
}