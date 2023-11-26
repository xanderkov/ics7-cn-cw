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

int wait_client(int server_socket)
{
	struct pollfd pollfds[MAX_CLIENTS + 1];
	pollfds[0].fd = server_socket;
	pollfds[0].events = POLLIN | POLLPRI;
	int useClient = 0;

	while (1)
	{
		// printf("useClient => %d\n", useClient);
		int pollResult = poll(pollfds, useClient + 1, 5000);
		if (pollResult > 0)
		{
			if (pollfds[0].revents & POLLIN)
			{
				struct sockaddr_in cliaddr;
				int addrlen = sizeof(cliaddr);
				int client_socket = accept(server_socket, (struct sockaddr*)&cliaddr, &addrlen);
				printf("accept success %s\n", inet_ntoa(cliaddr.sin_addr));
				for (int i = 1; i < MAX_CLIENTS; i++)
				{
					if (pollfds[i].fd == 0)
					{

						pollfds[i].fd = client_socket;
						pollfds[i].events = POLLIN | POLLPRI;
						useClient++;
						break;
					}
				}
			}
			for (int i = 1; i < MAX_CLIENTS; i++)
			{
				if (pollfds[i].fd > 0 && pollfds[i].revents & POLLIN)
				{
					char buf[SIZE];
					int bufSize = read(pollfds[i].fd, buf, SIZE - 1);
					if (bufSize == -1)
					{
						pollfds[i].fd = 0;
						pollfds[i].events = 0;
						pollfds[i].revents = 0;
						useClient--;
					}
					else if (bufSize == 0)
					{
						pollfds[i].fd = 0;
						pollfds[i].events = 0;
						pollfds[i].revents = 0;
						useClient--;
					}
					else
					{

						buf[bufSize] = '\0';
						printf("From client: %s\n", buf);
					}
				}
			}
		}
	}
}