#include "http.h"

int get_path_length(uint8_t *path) {
    int path_length = 0;
    // ignore the first character
    for (int i = 1; i < strlen(path); i++) {
        if (path[i] == '/') {
            path_length++;
        }
    }
    return path_length;
}


void parse_path(struct http_request *req) {
    // int path_length = get_path_length(req->url);
    // req->path = malloc(sizeof(struct path *) * path_length + 1);
    // req->path_size = path_length + 1;
    // int path_index = 0;
    // int path_start = 0;

    // for (int i = 1; i < strlen(req->url); i++) {
    //     if (req->url[i] == '/') {
    //         struct path *p = malloc(sizeof(struct path));
    //         p->name = malloc(i - path_start);
    //         strncpy(p->name, req->url + path_start, i - path_start);
    //         p->name[i - path_start] = '\0';
    //         p->name_size = i - path_start;
    //         req->path[path_index] = p;
    //         path_start = i + 1;
    //         path_index++;
    //     }
    // }

    // struct path *p = malloc(sizeof(struct path));
    // p->name = malloc(strlen(req->url) - path_start);
    // strncpy(p->name, 
}

struct http_request *parse_http_request(uint8_t *buffer, size_t buffer_size) {
    struct http_request *req = malloc(sizeof(struct http_request));
    int request_method_end = 0;
    // parse the request method
    if (strncmp(buffer, "GET", 3) == 0) {
        req->method = GET;
        request_method_end = 3;
    } else if (strncmp(buffer, "POST", 4) == 0) {
        req->method = POST;
        request_method_end = 4;
    } else if (strncmp(buffer, "PUT", 3) == 0) {
        req->method = PUT;
        request_method_end = 3;
    } else if (strncmp(buffer, "DELETE", 6) == 0) {
        req->method = DELETE;
        request_method_end = 6;
    } else if (strncmp(buffer, "PATCH", 5) == 0) {
        req->method = PATCH;
        request_method_end = 5;
    } else if (strncmp(buffer, "HEAD", 4) == 0) {
        req->method = HEAD;
        request_method_end = 4;
    } else if (strncmp(buffer, "OPTIONS", 7) == 0) {
        req->method = OPTIONS;
        request_method_end = 7;
    } else if (strncmp(buffer, "CONNECT", 7) == 0) {
        req->method = CONNECT;
        request_method_end = 7;
    } else if (strncmp(buffer, "TRACE", 5) == 0) {
        req->method = TRACE;
        request_method_end = 5;
    }

    // parse the request url
    uint8_t *url_start = buffer + request_method_end + 1;
    uint8_t *url_end = strchr(url_start, ' ');
    int url_size = url_end - url_start;
    req->url = malloc(url_size + 1);
    strncpy(req->url, url_start, url_size);
    req->url[url_size] = '\0';
    req->url_size = url_size + 1;
    printf("URL: %s\n", req->url);
    parse_path(req);

    return req;
}

void free_http_request(struct http_request *req) {
    free(req->url);
    for (int i = 0; i < req->path_size; i++) {
        free(req->path[i]->name);
        free(req->path[i]);
    }
    free(req->path);
    free(req);
}

void create_http_server(struct http_server *server) {
    server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->server_fd == -1) {
        printf("Socket creation failed: %s...\n", strerror(errno));
        exit(1);
    }

    if (server->reuse == 1) {
        if (setsockopt(server->server_fd, SOL_SOCKET, SO_REUSEPORT, &server->reuse, sizeof(server->reuse)) < 0) {
            printf("SO_REUSEPORT failed: %s \n", strerror(errno));
            exit(1);
        }
    }

    server->serv_addr.sin_family = AF_INET;
    server->serv_addr.sin_port = htons(server->port);
    server->serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server->server_fd, (struct sockaddr *) &server->serv_addr, sizeof(server->serv_addr)) != 0) {
        printf("Bind failed: %s \n", strerror(errno));
        exit(1);
    }

    if (listen(server->server_fd, server->connection_backlog) != 0) {
        printf("Listen failed: %s \n", strerror(errno));
        exit(1);
    }
}

void free_http_server(struct http_server *server) {
    close(server->server_fd);
}

struct http_response *create_http_response(uint16_t status_code, uint8_t *body, size_t body_size, struct http_header *headers, size_t headers_size, uint8_t *msg, size_t msg_size) {
    struct http_response *res = malloc(sizeof(struct http_response));
    res->status_code = status_code;
    if (body_size != 0) {
        res->body = malloc(body_size);
        strncpy(res->body, body, body_size);
        res->body_size = body_size;
    } else {
        res->body = malloc(1);
        strncpy(res->body, "\0", 1);
        res->body_size = 1;
    }
    if (headers_size != 0) {
        res->headers = headers;
    }
    res->headers_size = headers_size;
    if (msg_size != 0) {
        res->msg = malloc(msg_size);
        strncpy(res->msg, msg, msg_size);
        res->msg_size = msg_size;
    } else {
        res->msg = malloc(1);
        strncpy(res->msg, "\0", 1);
        res->msg_size = 1;
    }
    return res;
}

void free_http_response(struct http_response *res) {
    free(res->body);
    for(int i = 0; i < res->headers_size; i++) {
        free(res->headers[i].name);
        free(res->headers[i].value);
    }
    free(res->headers);
    free(res->msg);
    free(res);
}

uint8_t* http_response_to_string(struct http_response *res) {
    uint8_t *response = malloc(res->body_size + res->msg_size + 1024);
    sprintf(response, "HTTP/1.1 %d", res->status_code);
    if (res->msg_size != 0) {
        sprintf(response + strlen(response), " %s", res->msg);
    }
    for (int i = 0; i < res->headers_size; i++) {
        sprintf(response + strlen(response), "\r\n%s: %s", res->headers[i].name, res->headers[i].value);
    }
    sprintf(response + strlen(response), "\r\n%s", res->body);
    // print /0 at the end of the response
    sprintf(response + strlen(response), "\0");
    return response;
}