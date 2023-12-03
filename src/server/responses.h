//
// Created by Александр Ковель on 03.12.2023.
//

#ifndef SERVER_THREAD_SERVER_RESPONSES_H_
#define SERVER_THREAD_SERVER_RESPONSES_H_

#define OK_STR "HTTP/1.1 200 OK"
#define BAD_REQUEST_STR "HTTP/1.1 400 Bad Request\r\n\r\n"
#define FORBIDDEN_STR "HTTP/1.1 403 Forbidden\r\n\r\n"
#define NOT_FOUND_STR "HTTP/1.1 404 Not Found\r\n\r\n"
#define M_NOT_ALLOWED_STR "HTTP/1.1 405 Method Not Allowed\r\n\r\n"
#define INT_SERVER_ERR_STR "HTTP/1.1 500 Internal Server Error\r\n\r\n"

#endif //SERVER_THREAD_SERVER_RESPONSES_H_
