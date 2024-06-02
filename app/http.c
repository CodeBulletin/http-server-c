#include "http.h"
#include <zlib.h>
#include <pthread.h>

char *gzip_compress(char *input, size_t *len, size_t *outlen) {
    z_stream stream_s;
    stream_s.zalloc = Z_NULL;
    stream_s.zfree = Z_NULL;
    stream_s.opaque = Z_NULL;
    char *out = (char *)calloc(1024 * 1024, sizeof(char));
    stream_s.next_in = (Bytef *)input;
    stream_s.avail_in = *len;
    stream_s.next_out = (Bytef *)out;
    stream_s.avail_out = 1024 * 1024;
    deflateInit2(&stream_s, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 0x1f, 8,
                Z_DEFAULT_STRATEGY);
    deflate(&stream_s, Z_FINISH);
    deflateEnd(&stream_s);
    *outlen = stream_s.total_out;
    return out;
}

void to_hex(uint8_t *data, size_t data_size, uint8_t **hex_data, size_t *hex_data_size) {
    *hex_data = malloc(data_size * 2);
    for (size_t i = 0; i < data_size; i++) {
        sprintf(*hex_data + i * 2, "%02x", data[i]);
    }
    *hex_data_size = data_size * 2;
}


void parse_path(struct http_request *req) {
    size_t last_index = 1;
    size_t current_index = 1;
    size_t path_count = 0;
    req->path = NULL;
    struct http_path *current_path = NULL;
    
    for (; current_index < req->url_size - 1; current_index++) {
        if (req->url[current_index] == '/') {
            size_t path_size = current_index - last_index;
            if (path_size != 0) {
                struct http_path *new_path = malloc(sizeof(struct http_path));
                new_path->name = malloc(path_size + 1);
                strncpy(new_path->name, req->url + last_index, path_size);
                new_path->name[path_size] = '\0';
                new_path->name_size = path_size + 1;
                new_path->next = NULL;
                if (req->path == NULL) {
                    req->path = new_path;
                    current_path = new_path;
                } else {
                    current_path->next = new_path;
                    current_path = new_path;
                }
                path_count++;
            }
            last_index = current_index + 1;
        }
    }
    size_t path_size = current_index - last_index;
    if (path_size != 0) {
        struct http_path *new_path = malloc(sizeof(struct http_path));
        new_path->name = malloc(path_size + 1);
        strncpy(new_path->name, req->url + last_index, path_size);
        new_path->name[path_size] = '\0';
        new_path->name_size = path_size + 1;
        new_path->next = NULL;
        if (req->path == NULL) {
            req->path = new_path;
            current_path = new_path;
        } else {
            current_path->next = new_path;
            current_path = new_path;
        }
        path_count++;
    }
    req->path_size = path_count;
}

void *parse_http_headers(uint8_t *buffer, size_t buffer_size, struct http_request *req) {
    // Skip to the first /r/n
    uint8_t *current = buffer;
    while (current[0] != '\r' && current[1] != '\n') {
        current++;
    }  
    current += 2;

    req->headers = create_hashmap(100);
    
    // Split the headers by /r/n
    uint8_t *header = current;
    uint8_t *header_end;
    while (header[0] != '\r' && header[1] != '\n') {
        header_end = header;
        while (header_end[0] != '\r' && header_end[1] != '\n') {
            header_end++;
        }
        header_end += 2;
        if (header_end - header == 2) {
            break;
        }
        // Process the header
        uint8_t *header_name = header;
        uint8_t *header_name_end = strchr(header_name, ':');
        size_t header_name_size = header_name_end - header_name;
        uint8_t *header_value = header_name_end + 2;
        uint8_t *header_value_end = strchr(header_value, '\r');
        size_t header_value_size = header_value_end - header_value;

        uint8_t *name = malloc(header_name_size + 1);
        strncpy(name, header_name, header_name_size);
        name[header_name_size] = '\0';
        uint8_t *value = malloc(header_value_size + 1);
        strncpy(value, header_value, header_value_size);
        value[header_value_size] = '\0';

        insert(req->headers, name, value);

        free(name);
        free(value);

        header = header_end;
    }
}

struct http_request *parse_http_request(uint8_t *buffer, size_t buffer_size) {
    struct http_request *req = malloc(sizeof(struct http_request));
    size_t request_method_end = 0;
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
    size_t url_size = url_end - url_start;
    req->url = malloc(url_size + 1);
    strncpy(req->url, url_start, url_size);
    req->url[url_size] = '\0';
    req->url_size = url_size + 1;
    parse_path(req);

    // parse the request headers
    parse_http_headers(buffer, buffer_size, req);

    // parse the request body
    uint8_t *body_start = strstr(buffer, "\r\n\r\n");
    if (body_start != NULL) {
        body_start += 4;
        size_t body_size = buffer_size - (body_start - buffer);
        req->body = NULL;
        req->body_size = 0;
        if (body_size != 0) {
            req->body = malloc(body_size);
            strncpy(req->body, body_start, body_size);
            req->body_size = body_size;
        }
    } else {
        req->body = NULL;
        req->body_size = 0;
    }


    return req;
}

void free_http_request(struct http_request *req) {
    free(req->url);
    struct http_path *current = req->path;
    struct http_path *next;
    while (current != NULL) {
        next = current->next;
        free(current->name);
        free(current);
        current = next;
    }
    if (req->headers != NULL)
        free_hashmap(req->headers);
    if (req->body != NULL)
        free(req->body);
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

struct http_response *create_http_response(uint16_t status_code, uint8_t *body, size_t body_size, struct hashmap *headers, uint8_t *msg, size_t msg_size) {
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
    res->headers = headers;
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
    if (res->headers != NULL)
        free_hashmap(res->headers);
    free(res->msg);
    free(res);
}

uint8_t* http_response_to_string(struct http_response *res, size_t *response_size, size_t content_length) {
    uint8_t *response = malloc(1024 * 1024);
    sprintf(response, "HTTP/1.1 %d", res->status_code);
    if (res->msg_size != 0) {
        sprintf(response + strlen(response), " %s", res->msg);
    }

    // Insert content-length header
    if (get(res->headers, "Content-Length") == NULL) {
        uint8_t content_length_str[20];
        sprintf(content_length_str, "%ld", content_length);
        insert(res->headers, "Content-Length", content_length_str);
    }

    if (res->headers != NULL) {
        for (size_t i = 0; i < res->headers->capacity; i++) {
            struct map *current = res->headers->maps[i];
            while (current != NULL) {
                sprintf(response + strlen(response), "\r\n%s: %s", current->key, current->value);
                current = current->next;
            }
        }
    }

    sprintf(response + strlen(response), "\r\n");
    sprintf(response + strlen(response), "\r\n");
    *response_size = strlen(response);

    return response;
}

void content_response(struct http_response *res, int id, uint8_t **bytes_content, size_t *bytes_content_length) {
    uint8_t *content = NULL;
    uint8_t *content_text = NULL;
    size_t content_length = 0;

    if (get(res->headers, "Content-Encoding") == NULL) {
        content = res->body;
        content_length = res->body_size;

        content_text = malloc(content_length + 1);
        strncpy(content_text, content, content_length);
        content_text[content_length] = '\0';
    } else {
        size_t outlen = 0;
        content_text = gzip_compress(res->body, &res->body_size, &outlen);
        content_length = outlen;
    }

    *bytes_content = content_text;
    *bytes_content_length = content_length;
}

void send_response(int id, struct http_response *res) {
    size_t res_size;
    uint8_t *content = NULL;
    size_t content_length = 0;
    content_response(res, id, &content, &content_length);
    uint8_t *response = http_response_to_string(res, &res_size, content_length);

    // concatenate the response and the content
    uint8_t *response_content = malloc(res_size + content_length);
    strncpy(response_content, response, res_size);
    memcpy(response_content + res_size, content, content_length);

    send(id, response_content, res_size + content_length, 0);

    free(response_content);
    free(content);
    free(response);
}

char* request_method_to_string(enum http_request_type method) {
    switch (method) {
        case GET:
            return "GET";
        case POST:
            return "POST";
        case PUT:
            return "PUT";
        case DELETE:
            return "DELETE";
        case PATCH:
            return "PATCH";
        case HEAD:
            return "HEAD";
        case OPTIONS:
            return "OPTIONS";
        case CONNECT:
            return "CONNECT";
        case TRACE:
            return "TRACE";
    }
}

void handle_client(struct http_server *server, void *(*handle_request)(void *)) {
	while (1)
	{
        printf("Waiting for client...\n");
		struct sockaddr_in client_addr;

		int client_addr_len = sizeof(client_addr);
		
		int id = accept(server->server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
		printf("connected to client %d\n", id);

		pthread_t thread;
		int *client_id = &id;

		// handle_request(id, ok_res, not_found_res);
		pthread_create(&thread, NULL, handle_request, (void *) client_id);

		pthread_detach(thread);
	}
}