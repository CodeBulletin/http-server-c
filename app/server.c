
#include "http.h"
#include "helper.h"
#include <pthread.h>

void* handle_request(void *arg) {
	int id = *((int *) arg);
	// Ok Response
	struct http_response *ok_res = create_http_response(200, "", 0, NULL, "OK", 3);

	// Not Found Response
	struct http_response *not_found_res = create_http_response(404, "", 0, NULL, "Not Found", 10);

	uint8_t request_buffer[500];
	int bytes_received = read(id, request_buffer, sizeof(request_buffer));
	if(bytes_received < 0) {
		perror("Error receiving data from client");
		return NULL;
	}

	struct http_request *req = parse_http_request(request_buffer, bytes_received);

	printf("%s request at %s\n", request_method_to_string(req->method), req->url);

	struct path *current = req->path;
	if (req->method == GET && current == NULL) {
		send_response(id, ok_res);
	} else if (req->method == GET && current != NULL && strcmp(current->name, "echo") == 0) {
		current = current->next;
		if (current != NULL) {
			struct hashmap *headers = create_hashmap(2);
			insert(headers, "Content-Type", "text/plain");
			uint8_t *content_length = integer_to_sring(current->name_size - 1);
			insert(headers, "Content-Length", content_length);
			free(content_length);

			struct http_response *echo_res = create_http_response(200, current->name, current->name_size, headers, "OK", 3);
			send_response(id, echo_res);
			free_http_response(echo_res);
		} else {
			send_response(id, not_found_res);
		}
	} else if (req->method == GET && current != NULL && strcmp(current->name, "user-agent") == 0) {
		if (get(req->headers, "User-Agent") != NULL) {
			struct hashmap *headers = create_hashmap(2);
			insert(headers, "Content-Type", "text/plain");
			uint8_t *content_length = integer_to_sring(strlen(get(req->headers, "User-Agent")));
			insert(headers, "Content-Length", content_length);
			free(content_length);

			struct http_response *user_agent_res = create_http_response(200, get(req->headers, "User-Agent"), strlen(get(req->headers, "User-Agent")), headers, "OK", 3);
			send_response(id, user_agent_res);
			free_http_response(user_agent_res);
		} else {
			send_response(id, not_found_res);
		}
	} 
	else {
		send_response(id, not_found_res);
	}
	free_http_request(req);	
	free_http_response(ok_res);
	free_http_response(not_found_res);

	return NULL;
}

void handle_client(struct http_server *server) {
	while (1)
	{
		struct sockaddr_in client_addr;

		int client_addr_len = sizeof(client_addr);
		
		int id = accept(server->server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
		// printf("Client connected\n");

		pthread_t thread;
		int *client_id = &id;

		// handle_request(id, ok_res, not_found_res);
		pthread_create(&thread, NULL, handle_request, (void *) client_id);

		pthread_detach(thread);
	}
}

int main() {
	setbuf(stdout, NULL);

	// printf("Logs from your program will appear here!\n");

	struct http_server server = {
		.port = 4221,
		.connection_backlog = 20,
		.reuse = 1,
	};

	create_http_server(&server);

	handle_client(&server);

	free_http_server(&server);

	// printf("Connection closed\n");

	return 0;
}
