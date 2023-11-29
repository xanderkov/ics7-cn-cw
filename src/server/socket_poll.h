//
// Created by Александр Ковель on 26.11.2023.
//

#ifndef SERVER_THREAD_SERVER_SOCKET_POLL_H_
#define SERVER_THREAD_SERVER_SOCKET_POLL_H_

#include "logger.h"

#define SIZE 1024
#define MAX_CLIENTS 100

int creat_socket(int port);
int wait_client(int server_socket);

#endif //SERVER_THREAD_SERVER_SOCKET_POLL_H_
