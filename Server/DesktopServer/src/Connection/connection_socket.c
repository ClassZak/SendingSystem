/*
 * Sending System
 * Copyright (C) 2025-2026 ClassZak
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
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

