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
 * SQLite CRUD for the NamedConnection table.
 *
 * A NamedConnection is a user-visible label ("Home NAS") for a
 * Connection row.
 */
#pragma once

#include "model/database_models.h"
#include <sqlite3.h>




/**
 * Inserts a new NamedConnection row.
 *
 * @param db            (sqlite3*, Required) — open database.
 * @param name          (const char*, Optional) — display name.
 * @param connection_id (DB_ID, Required) — FK to Connection.id.
 * @param out_id        (DB_ID*, Optional) — receives the new row id.
 * @return (int) — 0 on success; -1 on SQL or FK error.
 */
int named_connection_insert(sqlite3* db, const char* name, DB_ID connection_id, DB_ID* out_id);




/**
 * Fetches a single NamedConnection by id.
 *
 * @param db (sqlite3*, Required) — open database.
 * @param id (DB_ID, Required) — row id.
 * @return (NamedConnection*) — heap struct, or NULL. Release with
 *         named_connection_free.
 */
NamedConnection* named_connection_get(sqlite3* db, DB_ID id);




/**
 * Fetches every NamedConnection row, ordered by id.
 *
 * @param db        (sqlite3*, Required) — open database.
 * @param out_count (int*, Required) — receives the number of rows.
 * @return (NamedConnection*) — heap array, or NULL when empty. Release
 *         with named_connection_free_all.
 */
NamedConnection* named_connection_get_all(sqlite3* db, int* out_count);




/**
 * Replaces the name and FK of an existing NamedConnection.
 *
 * @param db            (sqlite3*, Required) — open database.
 * @param id            (DB_ID, Required) — row id.
 * @param name          (const char*, Optional) — new display name.
 * @param connection_id (DB_ID, Required) — new FK to Connection.id.
 * @return (int) — 0 on success; -1 if not found or on SQL/FK error.
 */
int named_connection_update(sqlite3* db, DB_ID id, const char* name, DB_ID connection_id);




/**
 * Deletes a NamedConnection row.
 *
 * @param db (sqlite3*, Required) — open database.
 * @param id (DB_ID, Required) — row id.
 * @return (int) — 0 on success; -1 if not found or on SQL error.
 */
int named_connection_delete(sqlite3* db, DB_ID id);
