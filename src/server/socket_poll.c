//
// Created by Александр Ковель on 26.11.2023.
//

#include "socket_poll.h"

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

int creat_socket(int port)
{
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

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

int analyze_client_text(struct pollfd *pollfds)
{
	char buf[SIZE];
	ssize_t bufsize = read(pollfds->fd, buf, SIZE - 1);
	LOG_TRACE("Serving client %d", pollfds->fd);
	buf[bufsize] = '\0';
	LOG_TRACE("From client: %s\n", buf);
}

int read_from_socket(struct pollfd *pollfds, int numfds, int fd_index)
{
	int nread;
	ioctl(pollfds[fd_index].fd, FIONREAD, &nread);
	if (nread == 0)
	{
		close(pollfds[fd_index].fd);
		pollfds[fd_index].events = 0;
		LOG_TRACE("Removing client %d", pollfds[fd_index].fd);
		move_socket_after_close(pollfds, numfds, fd_index);
		numfds--;
	}
	else
		analyze_client_text(&pollfds[fd_index]);
	return numfds;
}

int poll_sockets(struct pollfd *pollfds, int numfds, int server_socket)
{
	poll(pollfds, numfds, -1);

	for (int fd_index = 0; fd_index < numfds; fd_index++)
	{
		if (pollfds[fd_index].revents & POLLIN)
			numfds = accept_socket(&pollfds[fd_index], numfds, server_socket);
		else
			numfds = read_from_socket(pollfds, numfds, fd_index);
	}
	return numfds;
}

int wait_client(int server_socket)
{
	struct pollfd pollfds[MAX_CLIENTS + 1];
	pollfds[0].fd = server_socket;
	pollfds[0].events = POLLIN | POLLPRI;
	int numfds = 1;

	while (1)
	{
		numfds = poll_sockets(pollfds, numfds, server_socket);
	}
}