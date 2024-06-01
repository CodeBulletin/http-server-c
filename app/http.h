#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

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


struct http_header {
    uint8_t *name;
    size_t name_size;
    uint8_t *value;
    size_t value_size;
};

struct path {
    uint8_t *name;
    size_t name_size;
    struct path *next;
};

struct http_request {
    enum http_request_type method;
    uint8_t *url;
    size_t url_size;
    struct http_header *headers;
    uint8_t *body;
    size_t body_size;
    struct path *path;
    size_t path_size;
};

struct http_response {
    uint16_t status_code;
    struct http_header **headers;
    size_t headers_size;
    uint8_t *body;
    size_t body_size;
    uint8_t *msg;
    size_t msg_size;
};


struct http_request *parse_http_request(uint8_t *buffer, size_t buffer_size);
void free_http_request(struct http_request *req);
void create_http_server(struct http_server *server);
void free_http_server(struct http_server *server);
struct http_response *create_http_response(uint16_t status_code, uint8_t *body, size_t body_size, struct http_header **headers, size_t headers_size, uint8_t *msg, size_t msg_size);
void free_http_response(struct http_response *res);
uint8_t* http_response_to_string(struct http_response *res);
struct http_header* create_http_header(uint8_t *name, size_t name_size, uint8_t *value, size_t value_size);
struct http_header** create_http_headers(size_t size, struct http_header *header[]);
void send_response(int id, struct http_response *res);
