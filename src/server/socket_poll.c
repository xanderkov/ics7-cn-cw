//
// Created by Александр Ковель on 26.11.2023.
//

#include "socket_poll.h"
#include "thread_pool.h"
#include "request.h"
#include "responses.h"
#include "content.h"

#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>


#define REQ_SIZE 2048
#define RESP_SIZE 1024

typedef struct worker_sock_t {
	char *wd;
	int *clientfd;
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
		return -1;
	}

	int listenResult = listen(server_socket, 5);
	if (listenResult == -1)
	{
		LOG_ERROR("listenResult");
		return -1;
	}
	LOG_INFO("SOCKET CREATED on host:port %s:%d!!!", host, port);
	return server_socket;
}

int read_req(char *buff, int clientfd) {
	long byte_read = 0;

	byte_read = read(clientfd, buff, REQ_SIZE - 1);
//	byte_read = recv(clientfd, buff, sizeof(buff), 0);
	if (byte_read <= 0)
	{

		return -1;
	}
	buff[byte_read - 1] = '\0';
	return 0;
}

int write_response(int fd, const void *buf, size_t n) {
	int byte_write = write(fd, buf, n);
	if (byte_write < 0)
	{
		LOG_ERROR("Write error clientfd: %d, thread: %d", fd, pthread_self());
	}
	return byte_write;
}

void send_err(int clientfd, const char *str) {
//	LOG_ERROR("ERROR %s", str);
//	LOG_ERROR( "  ЖОПА    "); // TODO: не забыть про Жопу
	write_response(clientfd, str, strlen(str));
}

char *get_type(char *path) {
	char *res = path + strlen(path) - 1;
	while (res >= path && *res != '.' && *res != '/')
	{
		res--;
	}

	if (res < path || *res == '/')
	{
		return NULL;
	}
	return ++res;
}


char *get_content_type(char *path) {
	char *ext = get_type(path);
	if (ext == NULL)
	{
		return NULL;
	}

	int i = 0;
	for (i = 0; i < TYPE_NUM && strcmp(TYPE_EXT[i], ext) != 0; i++);
	if (i >= TYPE_NUM) return NULL;

	return MIME_TYPE[i];
}


int send_headers(char *path, int clientfd) {
	char status[] = OK_STR,
		connection[] = "Connection: close";
	char *len = calloc(HEADER_LEN, sizeof(char)),
		*type = calloc(HEADER_LEN, sizeof(char));
	char *res_str = "/0";

	if (len == NULL || type == NULL) {
		LOG_ERROR("failed to alloc headers buffs");
		return -1;
	}

	struct stat st;
	if (stat(path, &st) < 0) {
		perror("stat error");
		return -1;
	}
	sprintf(len, "Content-Length: %lld", st.st_size);
	char *mime_type = get_content_type(path);

	int rc = 0;
	if (mime_type == NULL) {
		LOG_WARN("could not determine the file type");
		rc = asprintf(&res_str, "%s\r\n%s\r\n%s\r\n\r\n", status, connection, len);
	} else {
		sprintf(type, "Content-Type: %s", mime_type);
		rc = asprintf(&res_str, "%s\r\n%s\r\n%s\r\n%s\r\n\r\n", status, connection, len, type);
	}
	if (rc < 0)
	{
		LOG_ERROR("formation of headers of http response failed");
		return -1;
	}

	int byte_write = write_response(clientfd, res_str, rc);
	free(res_str);
	if (byte_write < 0)
	{
		return -1;
	}

	return 0;
}

void send_file(char *path, int clientfd) {
	int fd = open(path, 0);
	if (fd < 0) {
		LOG_ERROR("open error");
		return;
	}

	char *buff_resp = calloc(RESP_SIZE, sizeof(char));
	if (buff_resp == NULL) {
		LOG_ERROR("failed alloc resp buf");
		return;
	}
	unsigned long long total_read = 0, total_write = 0;
	long byte_write = 0, byte_read = 0;

	while ((byte_read = read(fd, buff_resp, RESP_SIZE)) > 0) {
		total_read += byte_read;

		byte_write = write(clientfd, buff_resp, byte_read);
		if (byte_write < 0) {
			LOG_ERROR("write error");
			break;
		}
		total_write += byte_write;
	}

	close(fd);
	LOG_INFO("successful response");
}

void process_get_req(char *path, int clientfd) {
	if (send_headers(path, clientfd) < 0)
		return;

	send_file(path, clientfd);
}

void process_head_req(char *path, int clientfd) {
	send_headers(path, clientfd);
}

void send_resp(char *path, int clientfd, request_method_t type) {
	switch (type) {
	case GET:
		process_get_req(path, clientfd);
		break;
	case HEAD:
		process_head_req(path, clientfd);
		break;
	default:
		LOG_ERROR("unsupported http method");
		send_err(clientfd, M_NOT_ALLOWED_STR);
		break;
	}
}


int is_prefix(char *prefix, char *str) {
	while (*prefix && *str && *prefix++ == *str++);

	if (*prefix == '\0')
	{
		return 1;
	}
	return 0;
}

void process_req(int clientfd, request_t *req, char *wd) {
	char *path = calloc(PATH_NUM, sizeof(char));
	if (path == NULL) {
		LOG_ERROR("Failed alloc path buf");
		return;
	}

	if (realpath(req->url, path) == NULL) {
		if (errno == ENOENT)
		{
			send_err(clientfd, NOT_FOUND_STR);
		}
		else
		{
			send_err(clientfd, INT_SERVER_ERR_STR);
		}
		LOG_ERROR("realpath error %s", strerror(errno));
		return;
	}

	if (!is_prefix(wd, path)) {
		send_err(clientfd, FORBIDDEN_STR);
		LOG_ERROR("attempt to access outside the root");
		return;
	}
	LOG_TRACE("clientfd: %d, thread: %d", clientfd, pthread_self());
	send_resp(path, clientfd, GET);
}


void worker(void* arg)
{
	worker_sock_t *worker_sock = arg;
	char *wd = worker_sock->wd;
	int *clientfd = worker_sock->clientfd;

	request_t req;
	char *buff = malloc(REQ_SIZE);
	if (buff == NULL)
	{
		LOG_ERROR("failed alloc req buf");
		return;
	}

	if (read_req(buff, *clientfd) < 0)
	{
		send_err(*clientfd, INT_SERVER_ERR_STR);
		close(*clientfd);
		return;
	}

	if (parse_req(&req, buff) < 0)
	{
		send_err(*clientfd, BAD_REQUEST_STR);
		close(*clientfd);
		return;
	}
	if (req.method == BAD)
	{
		LOG_ERROR("unsupported http method");
		send_err(*clientfd, M_NOT_ALLOWED_STR);
		close(*clientfd);
		return;
	}

	process_req(*clientfd, &req, wd);
	close(*clientfd);
	*clientfd = -1;
}



int wait_client(server_t *server)
{
	server->clients[0].fd = server->listen_sock;
	server->clients[0].events = POLLIN;
	int numfds = 0, maxcl = 10;
	int first = 0;

	while (1)
	{
		numfds = poll(server->clients, maxcl + 1, -1);

		if (numfds < 0)
		{
			LOG_ERROR("poll error");
			continue;
		}

		if (server->clients[0].revents & POLLIN)
		{
			int client_sock = accept(server->listen_sock, NULL, 0);
			if (client_sock < 0) continue;

			long i = 0;
			for (i = 1; i < server->cl_num; ++i)
			{
				if (server->clients[i].fd < 0)
				{
					server->clients[i].fd = client_sock;
					server->clients[i].events = POLLIN | POLLPRI;
					break;
				}
			}
			if (i == server->cl_num) {
				LOG_ERROR("too many connections");
				continue;
			}

			if (i > maxcl)
			{
				maxcl = i;
				LOG_INFO("Max clients: %d", maxcl);
			}
			if (--numfds <= 0)
			{
				continue;
			}
		}
		for (int i = 1; i <= maxcl; ++i)
		{
			if (server->clients[i].fd >= 0 && server->clients[i].revents & (POLLIN | POLLERR))
			{
				worker_sock_t worker_sock;
				worker_sock.clientfd = &server->clients[i].fd;
				worker_sock.wd = server->wd;

				tpool_add_work(server->pool, worker, &worker_sock);

				if (--numfds < 0)
				{
					break;
				}
			}
		}
		tpool_wait(server->pool);

	}
}