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

int	  message_insert(sqlite3* db, DB_ID connection_id, DB_ID status_id, DB_ID type_id,
						const char* response, const char* sending_time,
						const char* request, DB_ID* out_id);
Message* message_get(sqlite3* db, DB_ID id);
Message* message_get_all(sqlite3* db, int* out_count);
Message* message_get_last_successful(sqlite3* db);

int	  message_update(sqlite3* db, DB_ID id, DB_ID connection_id, DB_ID status_id, DB_ID type_id,
						const char* response, const char* sending_time, const char* request);
int	  message_delete(sqlite3* db, DB_ID id);
