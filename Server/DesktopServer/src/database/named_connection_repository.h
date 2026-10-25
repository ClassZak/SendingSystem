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

int					named_connection_insert(sqlite3* db, const char* name, DB_ID connection_id, DB_ID* out_id);
NamedConnection*	named_connection_get(sqlite3* db, DB_ID id);
NamedConnection*	named_connection_get_all(sqlite3* db, int* out_count);
int					named_connection_update(sqlite3* db, DB_ID id, const char* name, DB_ID connection_id);
int					named_connection_delete(sqlite3* db, DB_ID id);
