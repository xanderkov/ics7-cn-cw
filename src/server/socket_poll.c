//
// Created by Александр Ковель on 26.11.2023.
//

#include "socket_poll.h"
#include "thread_pool.h"

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

static const size_t num_threads = 10;

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


void worker(void* arg)
{

	struct pollfd *poll_fd = arg;

	char buf[SIZE];
	char method[SIZE];  /* request method */
	char uri[SIZE];     /* request uri */
	char version[SIZE]; /* request method */

	ssize_t bufsize = read(poll_fd->fd, buf, SIZE - 1);
	LOG_TRACE("Serving client %d", poll_fd->fd);

	buf[bufsize] = '\0';

	LOG_INFO("From client: %s", buf);

	sscanf(buf, "%s %s %s\n", method, uri, version);

	LOG_INFO("Method: %s", method);
}

int accept_socket(struct pollfd *poll_fd, int numfds, int server_socket)
{
	struct sockaddr_in cliaddr;
	int addrlen = sizeof(cliaddr);
	int client_socket = accept(server_socket, (struct sockaddr*)&cliaddr, &addrlen);
	LOG_INFO("accept success %s\n", inet_ntoa(cliaddr.sin_addr));
	poll_fd->fd = client_socket;
	poll_fd->events = POLLIN | POLLPRI;
	numfds++;
	return numfds;
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

int read_from_socket(struct pollfd *poll_fd, int numfds, tpool_t *tm)
{
	int nread;
	ioctl(poll_fd->fd, FIONREAD, &nread);
	if (nread == 0)
	{
		char response[] = "HTTP/1.0 200 OK\r\n"
						  "Content-Type: text/html; charset=UTF-8\r\n\r\n"
						  "<doctype !html><html><head><title></title>"
						  "<style>body"
						  "h1 { font-size:4cm; text-align: center; color: black;"
						  "}</style></head>"
						  "<body><h1>Not Found  (HTTP 404)</h1></body></html>\r\n";
		write(poll_fd->fd, response, strlen(response));
		close(poll_fd->fd);
		poll_fd->events = 0;
//		LOG_TRACE("Removing client %d", poll_fd->fd);
		numfds--;
	}
	else
		analyze_client_text(poll_fd, tm);
	return numfds;
}

void poll_sockets(struct pollfd *pollfds, int numfds, int server_socket, tpool_t *tm)
{
	poll(pollfds, numfds, -1);

	for (int fd_index = 0; fd_index < MAX_CLIENTS; fd_index++)
	{
		if (pollfds[fd_index].revents & POLLIN)
			numfds = accept_socket(&pollfds[fd_index], numfds, server_socket);
		else
			numfds = read_from_socket(&pollfds[fd_index], numfds, tm);
	}
}

int wait_client(int server_socket)
{
	struct pollfd pollfds[MAX_CLIENTS + 1];
	pollfds[0].fd = server_socket;
	pollfds[0].events = POLLIN | POLLPRI;
	int numfds = 1;

	tpool_t* tm;
	tm = tpool_create(num_threads);

	while (1)
		poll_sockets(pollfds, numfds, server_socket, tm);
	tpool_destroy(tm);
}