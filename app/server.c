
#include "http.c"

int main() {
	// Disable output buffering
	setbuf(stdout, NULL);

	// Ok Response
	struct http_response *ok_res = create_http_response(200, "", 0, NULL, 0, "OK", 2);
	uint8_t *ok_res_str = http_response_to_string(ok_res);
	size_t ok_res_str_len = strlen(ok_res_str);
	printf("%s\n", ok_res_str);
	printf("%zu\n", ok_res_str_len);

	// Not Found Response
	struct http_response *not_found_res = create_http_response(404, "", 0, NULL, 0, "Not Found", 9);
	uint8_t *not_found_res_str = http_response_to_string(not_found_res);\
	size_t not_found_res_str_len = strlen(not_found_res_str);
	printf("%s\n", not_found_res_str);
	printf("%zu\n", not_found_res_str_len);


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

	if (req->method == GET && strcmp(req->url, "/") == 0) {
		send(id, ok_res_str, strlen(ok_res_str), 0);
	} else {
		send(id, not_found_res_str, strlen(not_found_res_str), 0);
	}

	// close the connection
	free_http_request(req);
	free_http_response(ok_res);
	free_http_response(not_found_res);
	free(ok_res_str);
	free(not_found_res_str);
	free_http_server(&server);

	return 0;
}
