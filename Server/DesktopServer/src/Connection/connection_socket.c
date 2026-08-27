#include "connection_socket.h"
#include <unistd.h>
#include <stdlib.h>



size_t connection_read_socket(void* ctx, void* buf, size_t len) {
	connection_socket_ctx* socket_ctx = (connection_socket_ctx*)ctx;
	size_t received = recv(socket_ctx->client_socket_fd, buf, len, 0);
	
	return received;
}


size_t connection_write_socket(void* ctx, void* buf, size_t len) {
	connection_socket_ctx* socket_ctx = (connection_socket_ctx*)ctx;
	size_t sent = send(socket_ctx->client_socket_fd, buf, len, 0);

	return sent;
}


void connection_close_socket(void* ctx) {
	connection_socket_ctx* socket_ctx = (connection_socket_ctx*)ctx;
	close(socket_ctx->client_socket_fd);
}


int connection_init_socket(void* ctx) {
	return EXIT_SUCCESS;
}

