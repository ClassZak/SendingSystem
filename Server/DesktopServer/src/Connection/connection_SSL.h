#pragma once
#include "connection.h"
#include <stddef.h>
#include <openssl/ssl.h>
#ifndef _WIN32
#include <netinet/in.h>
#else
#include <ws2def.h>
#endif



typedef struct connection_SSL_ctx {
	struct sockaddr_in client_address;
	SSL_CTX* ctx;
	SSL* ssl;
	int client_socket_fd;
} connection_SSL_ctx;


int connection_init_SSL(void* ctx);
size_t connection_read_SSL(void* ctx, void* buf, size_t len);
size_t connection_write_SSL(void* ctx, void* buf, size_t len);
void connection_close_SSL(void* ctx);



static inline struct connection_t create_connection_SSL() {
	struct connection_t result;
	result.ctx = NULL;
	result.init = &connection_init_SSL;
	result.read = &connection_read_SSL;
	result.write = &connection_write_SSL;
	result.close = &connection_close_SSL;

	return result;
}
