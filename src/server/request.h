//
// Created by Александр Ковель on 03.12.2023.
//

#ifndef SERVER_THREAD_SERVER_REQUEST_H_
#define SERVER_THREAD_SERVER_REQUEST_H_

#define URL_LEN 128

#define GET_STR "GET"
#define HEAD_STR "HEAD"

#define HTTP11_STR "HTTP/1.1"
#define HTTP10_STR "HTTP/1.0"

typedef enum request_method_t {
	BAD, GET, HEAD
} request_method_t;


typedef struct request_t {
	request_method_t method;
	char url[URL_LEN];
} request_t;

int parse_req(request_t *req, char *buff);

#endif //SERVER_THREAD_SERVER_REQUEST_H_
