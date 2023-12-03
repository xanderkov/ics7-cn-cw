//
// Created by Александр Ковель on 03.12.2023.
//

#include "request.h"
#include "logger.h"

#include <string.h>
#include <errno.h>

request_method_t parse_method(char *method) {
	if (strcmp(GET_STR, method) == 0)
	{
		return GET;
	}
	if (strcmp(HEAD_STR, method) == 0)
	{
		return HEAD;
	}
	return BAD;
}

int validate_version(char *version) {
	if (strcmp(HTTP11_STR, version) == 0 || strcmp(HTTP10_STR, version) == 0) return 1;
	return 0;
}

int parse_req(request_t *req, char *buff)
{
    char *saveptr = NULL;

    char *http_query = strtok_r(buff, "\n", &saveptr);
    if (http_query == NULL)
    {
		LOG_ERROR("Failed parse http query string");
		return -1;
    }
    LOG_INFO(http_query);

    char *http_method = strtok_r(http_query, " ", &saveptr);
    if (http_method == NULL)
    {
		LOG_ERROR("Failed parse http method");
		return -1;
    }
    req->method = parse_method(http_method);
    if (req->method == BAD)
    {
		return 0;
    }

    char *http_url = strtok_r(NULL, " ", &saveptr);
    if (http_url == NULL)
	{
		LOG_ERROR("failed parse url");
		return -1;
    }
    strcpy(req->url, http_url + 1);

    char *http_version = strtok_r(NULL, "\r", &saveptr); //todo version as enum
    if (http_version == NULL)
	{
		LOG_ERROR("failed http version");
		return -1;
    }
    if (validate_version(http_version) == 0)
	{
		LOG_ERROR("unsupported http version");
		return -1;
    }

    return 0;
}

