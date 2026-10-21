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
#include "connection.h"
#include "../../PrintProcedures/PrintProcedures.h"
#include "../database_config.h"
#include <sqlite3.h>
#include <stdlib.h>


struct Connection* get_connections_from_data_base(uint32_t* count) {
	struct Connection* connections = NULL;
	uint32_t connections_count = 0UL;

	sqlite3* db;
	char* err_msg = 0;
	int res;


	if (sqlite3_open(DATABASE_FILENAME, &db)) {
		print_error("Failed to open data base: %s\n", sqlite3_errmsg(db));

		return NULL;
	}

	

	
	sqlite3_close(db);


	return connections;
}

