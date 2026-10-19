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

