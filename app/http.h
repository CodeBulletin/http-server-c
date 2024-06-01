#ifndef HTTP_H
#define HTTP_H

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <errno.h>
#include <unistd.h>

#include "map.h"

#define MAX_HEADER_SIZE 1024
#define MAX_BODY_SIZE 1024
#define MAX_URL_SIZE 1024
#define MAX_EXTRA_SIZE 512

enum http_request_type {
    GET,
    POST,
    PUT,
    DELETE,
    PATCH,
    HEAD,
    OPTIONS,
    CONNECT,
    TRACE
};

struct http_server
{
    int server_fd;
    struct sockaddr_in serv_addr;
    int connection_backlog;
    int reuse;
    uint16_t port;
};

struct path {
    uint8_t *name;
    size_t name_size;
    struct path *next;
};

struct http_request {
    enum http_request_type method;
    struct hashmap *headers;
    uint8_t *url;
    size_t url_size;
    uint8_t *body;
    size_t body_size;
    struct path *path;
    size_t path_size;
};

struct http_response {
    uint16_t status_code;
    struct hashmap *headers;
    uint8_t *body;
    size_t body_size;
    uint8_t *msg;
    size_t msg_size;
};

struct http_request *parse_http_request(uint8_t *buffer, size_t buffer_size);
void create_http_server(struct http_server *server);
struct http_response *create_http_response(uint16_t status_code, uint8_t *body, size_t body_size, struct hashmap *headers, uint8_t *msg, size_t msg_size);
uint8_t* http_response_to_string(struct http_response *res);
void send_response(int id, struct http_response *res);

void free_http_request(struct http_request *req);
void free_http_server(struct http_server *server);
void free_http_response(struct http_response *res);

char* request_method_to_string(enum http_request_type method);

#endif