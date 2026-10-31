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
 * Saves connection to database.
 * Uses static variables from config.h
 */
#include "save_connection.h"


#include "../util/config.h"
#include "../PrintProcedures/PrintProcedures.h"

#include "../database/connection_repository.h"
#include "../database/Database.h"

#include "../util/config.h"




/**
 * Saves new connection to data base
 * Any - Ok
 * -1 - Failed
 * */
DB_ID save_connection(struct mg_connection* conn) {
	const struct mg_request_info* request_info = mg_get_request_info(conn);
	sqlite3* database;
	if (database_open(DATABASE_FILENAME, &database) == -1) {
		return -1;
	}

	DB_ID element = connection_get_or_create(database, request_info->remote_addr, CONNECTION_PORT, SSL_ENCRYPT, INCOMING_PEER_ROLE);

	database_close(database);

	if (element == -1) {
		print_error("Failed to save connection\n");
		print_error("Error in file '%s', line:%d in '%s'\n", __FILE__, __LINE__, __FUNCTION__);

		return -1;
	}

	return element;
}

