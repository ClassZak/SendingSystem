#include <unistd.h>
#include "connection_SSL.h"
#include "../OpenSSL/OpenSSL.h"




int connection_init_SSL(void* ctx) {
	connection_SSL_ctx* connection_ctx = (connection_SSL_ctx*)ctx;
	if ((connection_ctx->ssl = SSL_new(connection_ctx->ctx)) == NULL) {
		print_all_SSL_errors();
		return EXIT_FAILURE;
	}
	if (1 != (SSL_set_fd(connection_ctx->ssl, connection_ctx->client_socket_fd))) {
		print_all_SSL_errors();
		return EXIT_FAILURE;
	}
	if (SSL_accept(connection_ctx->ssl) <= 0) {
		print_all_SSL_errors();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


size_t connection_read_SSL(void* ctx, void* buf, size_t len) {
	connection_SSL_ctx* connection_ctx = (connection_SSL_ctx*)ctx;
	size_t received = SSL_read(connection_ctx->ssl, buf, len);

	return received;
}


size_t connection_write_SSL(void* ctx, void* buf, size_t len) {
	connection_SSL_ctx* connection_ctx = (connection_SSL_ctx*)ctx;
	size_t sent = SSL_write(connection_ctx->ssl, buf, len);

	return sent;
}


void connection_close_SSL(void* ctx) {
	connection_SSL_ctx* connection_ctx = (connection_SSL_ctx*)ctx;
	close(connection_ctx->client_socket_fd);
	SSL_shutdown(connection_ctx->ssl);
	SSL_free(connection_ctx->ssl);
}

