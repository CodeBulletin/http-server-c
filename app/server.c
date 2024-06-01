
#include "http.h"

int main() {
	// Disable output buffering
	setbuf(stdout, NULL);

	// Ok Response
	struct http_response *ok_res = create_http_response(200, "", 0, NULL, 0, "OK", 3);

	// Not Found Response
	struct http_response *not_found_res = create_http_response(404, "", 0, NULL, 0, "Not Found", 10);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");

	struct http_server server = {
		.port = 4221,
		.connection_backlog = 5,
		.reuse = 1,
	};

	create_http_server(&server);
	
	struct sockaddr_in client_addr;

	printf("Waiting for a client to connect...\n");
	int client_addr_len = sizeof(client_addr);
	
	int id = accept(server.server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
	printf("Client connected\n");

	uint8_t request_buffer[MAX_HEADER_SIZE + MAX_BODY_SIZE + MAX_URL_SIZE + MAX_EXTRA_SIZE];
	int bytes_received = recv(id, request_buffer, sizeof(request_buffer), 0);

	struct http_request *req = parse_http_request(request_buffer, bytes_received);

	struct path *current = req->path;
	if (req->method == GET && current == NULL) {
		send_response(id, ok_res);
	} else if (req->method == GET && current != NULL && strcmp(current->name, "echo") == 0) {
		current = current->next;
		if (current != NULL) {
			struct http_header *ct = create_http_header("Content-Type", 13, "text/plain", 11);
			char str[20];
			sprintf(str, "%d", current->name_size - 1);
			struct http_header *cl = create_http_header("Content-Length", 15, str, current->name_size - 1);

			struct http_header **headers = create_http_headers(2, (struct http_header *[]) {ct, cl});

			struct http_response *echo_res = create_http_response(200, current->name, current->name_size, headers, 2, "OK", 3);
			send_response(id, echo_res);
			free_http_response(echo_res);
		} else {
			send_response(id, not_found_res);
		}
	} else {
		send_response(id, not_found_res);
	}

	// close the connection
	free_http_request(req);
	free_http_response(ok_res);
	free_http_response(not_found_res);
	free_http_server(&server);

	printf("Connection closed\n");

	return 0;
}
