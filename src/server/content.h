//
// Created by Александр Ковель on 03.12.2023.
//

#ifndef SERVER_THREAD_SERVER_CONTENT_H_
#define SERVER_THREAD_SERVER_CONTENT_H_

#define TYPE_NUM 9

extern char *TYPE_EXT[TYPE_NUM];
extern char *MIME_TYPE[TYPE_NUM];

char *TYPE_EXT[TYPE_NUM] = {"txt", "css", "html", "js", "png", "jpg", "jpeg", "swf", "gif"};
char *MIME_TYPE[TYPE_NUM] = {"text/plain", "text/css", "text/html", "text/javascript", "image/png", "image/jpeg",
							 "image/jpeg", "application/x-shockwave-flash", "image/gif"};

#endif //SERVER_THREAD_SERVER_CONTENT_H_
