#pragma once
#include "connection.h"
#include <stddef.h>
#ifndef _WIN32
#include <netinet/in.h>
#else
#include <ws2def.h>
#endif


typedef struct connection_socket_ctx {
	struct sockaddr_in client_address;
	int client_socket_fd;
} connection_socket_ctx;


int connection_init_socket(void* ctx);
size_t connection_read_socket(void* ctx, void* buf, size_t len);
size_t connection_write_socket(void* ctx, void* buf, size_t len);
void connection_close_socket(void* ctx);


static inline struct connection_t create_connection_socket() {
	struct connection_t result;
	result.ctx = NULL;
	result.init = &connection_init_socket;
	result.read = &connection_read_socket;
	result.write = &connection_write_socket;
	result.close = &connection_close_socket;

	return result;
}

