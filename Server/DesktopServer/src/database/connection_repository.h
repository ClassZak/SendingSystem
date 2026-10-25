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
 * SQLite CRUD for the Connection table.
 */
#pragma once

#include "model/database_models.h"
#include <sqlite3.h>




/**
 * Inserts a new Connection row.
 *
 * @param db     (sqlite3*, Required) — open database.
 * @param ip     (const char*, Optional) — IP address or hostname.
 * @param port   (int, Required) — TCP port.
 * @param ssl    (int, Required) — 1 if TLS, 0 otherwise.
 * @param out_id (DB_ID*, Optional) — receives the new row id.
 * @return (int) — 0 on success; -1 on SQL error.
 */
int connection_insert(sqlite3* db, const char* ip, int port, int ssl, DB_ID* out_id);




/**
 * Fetches a single Connection by id.
 *
 * @param db (sqlite3*, Required) — open database.
 * @param id (DB_ID, Required) — row id.
 * @return (Connection*) — heap struct, or NULL. Release with
 *         connection_free.
 */
Connection* connection_get(sqlite3* db, DB_ID id);




/**
 * Fetches every Connection row, ordered by id.
 *
 * @param db        (sqlite3*, Required) — open database.
 * @param out_count (int*, Required) — receives the number of rows.
 * @return (Connection*) — heap array, or NULL when empty. Release
 *         with connection_free_all.
 */
Connection* connection_get_all(sqlite3* db, int* out_count);




/**
 * Finds the row matching (ip, port, ssl) or inserts a new one.
 *
 * @param db   (sqlite3*, Required) — open database.
 * @param ip   (const char*, Optional) — IP address or hostname.
 * @param port (int, Required) — TCP port.
 * @param ssl  (int, Required) — 1 if TLS, 0 otherwise.
 * @return (DB_ID) — existing or new row id; -1 on SQL error.
 */
DB_ID connection_get_or_create(sqlite3* db, const char* ip, int port, int ssl);




/**
 * Replaces ip / port / ssl of an existing Connection.
 *
 * @param db   (sqlite3*, Required) — open database.
 * @param id   (DB_ID, Required) — row id.
 * @param ip   (const char*, Optional) — new IP address.
 * @param port (int, Required) — new port.
 * @param ssl  (int, Required) — new SSL flag.
 * @return (int) — 0 on success; -1 if not found or on SQL error.
 */
int connection_update(sqlite3* db, DB_ID id, const char* ip, int port, int ssl);




/**
 * Deletes a Connection row.
 *
 * @param db (sqlite3*, Required) — open database.
 * @param id (DB_ID, Required) — row id.
 * @return (int) — 0 on success; -1 on SQL error or if another row
 *         still references this id.
 */
int connection_delete(sqlite3* db, DB_ID id);
