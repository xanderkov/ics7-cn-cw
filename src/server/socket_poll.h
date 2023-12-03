//
// Created by Александр Ковель on 26.11.2023.
//

#ifndef SERVER_THREAD_SERVER_SOCKET_POLL_H_
#define SERVER_THREAD_SERVER_SOCKET_POLL_H_

#include "logger.h"
#include "server.h"

#define SIZE 1024
#define MAX_CLIENTS 100

int creat_socket(int port, char *host);
int wait_client(server_t *server);

#endif //SERVER_THREAD_SERVER_SOCKET_POLL_H_
