
#include "http.h"
#include "helper.h"
#include <pthread.h>

char* directory = NULL;

void echo(int id, uint8_t *data, size_t data_size, struct hashmap *headers) {
	// Bad Request Response
	struct hashmap *bad_request_headers = create_hashmap(2);
	insert(bad_request_headers, "Content-Type", "text/plain");
	insert(bad_request_headers, "Content-Length", "25");
	struct http_response *bad_request_res = create_http_response(400, "Bad Request Echo is Empty", 26, bad_request_headers, "Bad Request", 12);

	if (data_size == 0) {
		send_response(id, bad_request_res);
		free_http_response(bad_request_res);
		return;
	}

	struct hashmap *ok_headers = create_hashmap(2);
	insert(ok_headers, "Content-Type", "text/plain");
	uint8_t *content_length = integer_to_sring(data_size);
	insert(ok_headers, "Content-Length", content_length);
	uint8_t *encoding_accepted = get(headers, "Accept-Encoding");
	if (encoding_accepted != NULL && strstr(encoding_accepted, "gzip") != NULL) {
		insert(ok_headers, "Content-Encoding", "gzip");
	}
	free(content_length);

	struct http_response *echo_res = create_http_response(200, data, data_size, ok_headers, "OK", 3);
	send_response(id, echo_res);
	free_http_response(echo_res);
	free_http_response(bad_request_res);
}

void userAgent(int id, uint8_t *data, size_t data_size, struct hashmap *headers) {
	// Bad Request Response
	struct hashmap *bad_request_headers = create_hashmap(2);
	insert(bad_request_headers, "Content-Type", "text/plain");
	insert(bad_request_headers, "Content-Length", "25");
	struct http_response *bad_request_res = create_http_response(400, "Bad Request User-Agent is Empty", 30, bad_request_headers, "Bad Request", 12);

	if (data == NULL || data_size == 0) {
		send_response(id, bad_request_res);
		free_http_response(bad_request_res);
		return;
	}

	struct hashmap *ok_headers = create_hashmap(2);
	insert(ok_headers, "Content-Type", "text/plain");
	uint8_t *content_length = integer_to_sring(data_size);
	insert(ok_headers, "Content-Length", content_length);	
	uint8_t *encoding_accepted = get(headers, "Accept-Encoding");
	if (encoding_accepted != NULL && strstr(encoding_accepted, "gzip") != NULL) {
		insert(ok_headers, "Content-Encoding", "gzip");
	}
	free(content_length);

	struct http_response *user_agent_res = create_http_response(200, data, data_size, ok_headers, "OK", 3);
	send_response(id, user_agent_res);
	free_http_response(user_agent_res);
	free_http_response(bad_request_res);
}

void getFile(int id, uint8_t *data, size_t data_size, struct hashmap *headers) {
	// Internal Server Error Response
	struct hashmap *internal_server_error_headers = create_hashmap(2);
	insert(internal_server_error_headers, "Content-Type", "text/plain");
	insert(internal_server_error_headers, "Content-Length", "21");
	struct http_response *internal_server_error_res = create_http_response(500, "Internal Server Error", 22, internal_server_error_headers, "Internal Server Error", 22);

	if (directory == NULL) {
		send_response(id, internal_server_error_res);
		free_http_response(internal_server_error_res);
		return;
	}

	// Bad Request Response
	struct hashmap *bad_request_headers = create_hashmap(2);
	insert(bad_request_headers, "Content-Type", "text/plain");
	insert(bad_request_headers, "Content-Length", "25");
	struct http_response *bad_request_res = create_http_response(400, "Bad Request Files is Empty", 26, bad_request_headers, "Bad Request", 11);

	if (data_size == 0) {
		send_response(id, bad_request_res);
		free_http_response(bad_request_res);
		return;
	}

	// Not Found Response
	struct hashmap *not_found_headers = create_hashmap(2);
	insert(not_found_headers, "Content-Type", "text/plain");
	insert(not_found_headers, "Content-Length", "0");
	struct http_response *not_found_res = create_http_response(404, "", 0, not_found_headers, "Not Found", 10);

	// Search for the files in directory and send the file
	uint8_t *file_path = malloc(data_size + strlen(directory) + 1);
	strncpy(file_path, directory, strlen(directory));
	strncpy(file_path + strlen(directory), data, data_size);
	file_path[data_size + strlen(directory)] = '\0';

	FILE *file = fopen(file_path, "r");
	if (file == NULL) {
		send_response(id, not_found_res);
		free(file_path);
		free_http_response(internal_server_error_res);
		free_http_response(bad_request_res);
		free_http_response(not_found_res);
		return;
	}

	fseek(file, 0, SEEK_END);
	size_t file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	uint8_t *file_data = malloc(file_size);
	fread(file_data, 1, file_size, file);
	fclose(file);

	struct hashmap *ok_headers = create_hashmap(2);
	insert(ok_headers, "Content-Type", "application/octet-stream");
	insert(ok_headers, "Content-Length", integer_to_sring(file_size));
	uint8_t *encoding_accepted = get(headers, "Accept-Encoding");
	if (encoding_accepted != NULL && strstr(encoding_accepted, "gzip") != NULL) {
		insert(ok_headers, "Content-Encoding", "gzip");
	}
	size_t content_length = file_size;

	struct http_response *file_res = create_http_response(200, file_data, file_size, ok_headers, "OK", 3);
	send_response(id, file_res);

	free(file_path);
	free_http_response(internal_server_error_res);
	free_http_response(bad_request_res);
	free_http_response(not_found_res);
}

void postFile(int id, uint8_t *data, size_t data_size, uint8_t *path, size_t path_size) {
	// Internal Server Error Response
	struct hashmap *internal_server_error_headers = create_hashmap(2);
	insert(internal_server_error_headers, "Content-Type", "text/plain");
	insert(internal_server_error_headers, "Content-Length", "21");
	struct http_response *internal_server_error_res = create_http_response(500, "Internal Server Error", 22, internal_server_error_headers, "Internal Server Error", 22);

	if (directory == NULL) {
		send_response(id, internal_server_error_res);
		free_http_response(internal_server_error_res);
		return;
	}

	uint8_t *file_path = malloc(path_size + strlen(directory) + 1);
	strncpy(file_path, directory, strlen(directory));
	strncpy(file_path + strlen(directory), path, path_size);
	file_path[path_size + strlen(directory)] = '\0';

	// Create a txt file and write the data
	FILE *file = fopen(file_path, "w");
	if (file == NULL) {
		send_response(id, internal_server_error_res);
		free(file_path);
		free_http_response(internal_server_error_res);
		return;
	}

	printf("File Path: %s\n", file_path);
	printf("Data: %s\n", data);
	printf("Data Size: %ld\n", data_size);

	fwrite(data, 1, data_size, file);
	fclose(file);

	// Created Response
	struct hashmap *ok_headers = create_hashmap(2);
	insert(ok_headers, "Content-Type", "text/plain");
	insert(ok_headers, "Content-Length", "25");
	struct http_response *ok_res = create_http_response(201, "File Created Successfully", 26, ok_headers, "Created", 8);

	send_response(id, ok_res);

	free(file_path);
	free_http_response(internal_server_error_res);
	free_http_response(ok_res);
}

void* handle_request(void *arg) {
	// Ok Response
	struct hashmap *ok_headers = create_hashmap(2);
	insert(ok_headers, "Content-Type", "text/plain");
	insert(ok_headers, "Content-Length", "0");
	struct http_response *ok_res = create_http_response(200, "", 0, ok_headers, "OK", 3);

	// Not Found Response
	struct hashmap *not_found_headers = create_hashmap(2);
	insert(not_found_headers, "Content-Type", "text/plain");
	insert(not_found_headers, "Content-Length", "0");
	struct http_response *not_found_res = create_http_response(404, "", 0, not_found_headers, "Not Found", 10);


	int id = *((int *) arg);
	uint8_t* request_buffer = malloc(1024 * 1024);
	int bytes_received = recv(id, request_buffer, 1024 * 1024, 0);
	if(bytes_received < 0) {
		perror("Error receiving data from client");
		return NULL;
	}
	struct http_request *req = parse_http_request(request_buffer, bytes_received);
	printf("%s request at %s\n", request_method_to_string(req->method), req->url);
	struct http_path *current = req->path;

	if (req->method == GET && current == NULL) {
		send_response(id, ok_res);
	} else if (req->method == GET && current != NULL && strcmp(current->name, "echo") == 0) {
		current = current->next;
		uint8_t *data =  NULL;
		size_t data_size = 0;
		if (current != NULL) {
			data = current->name;
			data_size = current->name_size - 1;
		}
		echo(id, data, data_size, req->headers);
	} else if (req->method == GET && current != NULL && strcmp(current->name, "user-agent") == 0) {
		int length = 0;
		uint8_t *user_agent = get(req->headers, "User-Agent");
		if (user_agent != NULL) {
			length = strlen(user_agent);
		}
		userAgent(id, user_agent, length, req->headers);
	} else if (req->method == GET && current != NULL && strcmp(current->name, "files") == 0) {
		getFile(id, req->url + 7, req->url_size - 7, req->headers);
	} else if (req->method == POST && current != NULL && strcmp(current->name, "files") == 0) {
		postFile(id, req->body, req->body_size, req->url + 7, req->url_size - 7);
	}
	else {
		send_response(id, not_found_res);
	}

	printf("Request handled %s:%s\n", req->url, request_method_to_string(req->method));

	free_http_request(req);	
	free_http_response(ok_res);
	free_http_response(not_found_res);

	printf("Connection closed %d\n", id);

	return NULL;
}

int main(int argc, char *argv[]) {
	setbuf(stdout, NULL);

	if (argc >= 2) {
		if (strcmp(argv[1], "--directory") == 0) {
			directory = argv[2];
		}
	}

	// printf("Logs from your program will appear here!\n");

	struct http_server server = {
		.port = 4221,
		.connection_backlog = 20,
		.reuse = 1,
	};

	create_http_server(&server);

	handle_client(&server, handle_request);

	free_http_server(&server);

	// printf("Connection closed\n");

	return 0;
}
