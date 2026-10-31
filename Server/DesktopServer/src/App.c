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
/**
 * Build-flag placeholder for a future dual-purpose binary.
 *
 * Currently unused: the server and the client build as separate
 * executables. Reserved for the moment when a single binary picks its
 * role at startup.
 */

#include <stdbool.h>

#include <libterm/libterm.h>
#include <stdlib.h>
#include <stdint.h>

#include "PrintProcedures/PrintProcedures.h"

#include "Server.h"
#include "Client.h"




struct arg_params_t {
	bool is_server;
	uint16_t connection_port;
	char* connection_ip;
};

int parse_commands();

int main(int argc, char** argv) {
	if (lt_init() != LT_OK) {
		print_error("Error console UI initialization\n");
		print_error("Error in file '%s', line:%d in '%s'\n", __FILE__, __LINE__, __FUNCTION__);
		return EXIT_FAILURE;
	}

	int x = 2, y = 1;
	lt_print(x, y, LT_GREEN, LT_DEFAULT, "q to quit");
	lt_present();

	struct lt_event event;
	while (lt_poll_event(&event) == LT_OK) {
		if ((event.type = LT_EVENT_KEY) && (event.ch == 'q')) {
			break;
		} else if (event.key == LT_KEY_ARROW_RIGHT) {
			++x;
			lt_print(x, y, LT_GREEN, LT_DEFAULT, "q to quit");
			lt_present();
		}
	}

	lt_shutdown();

	if (server() == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

