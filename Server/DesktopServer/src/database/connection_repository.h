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
#include "model/database_models.h"
#include <sqlite3.h>




int		 connection_insert(sqlite3* db, const char* ip, int port, int ssl, DB_ID* out_id);
Connection* connection_get(sqlite3* db, DB_ID id);
Connection* connection_get_all(sqlite3* db, int* out_count);


/* Find or insert by (ip, port, ssl). Returns id, or -1 on failure. */
DB_ID	   connection_get_or_create(sqlite3* db, const char* ip, int port, int ssl);


int		 connection_update(sqlite3* db, DB_ID id, const char* ip, int port, int ssl);
int		 connection_delete(sqlite3* db, DB_ID id);
