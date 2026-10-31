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
 * SQLite CRUD for the MessageStatus table.
 */
#pragma once

#include "model/database_models.h"
#include <sqlite3.h>




/**
 * Inserts a new MessageStatus row.
 *
 * @param db     (sqlite3*, Required) — open database.
 * @param name   (const char*, Required) — status name, e.g. "SUCCESS".
 * @param out_id (DB_ID*, Optional) — receives the new row id.
 * @return (int) — 0 on success; -1 on SQL error.
 */
int message_status_insert(sqlite3* db, const char* name, DB_ID* out_id);




/**
 * Fetches a single MessageStatus by id.
 *
 * @param db (sqlite3*, Required) — open database.
 * @param id (DB_ID, Required) — row id.
 * @return (MessageStatus*) — heap struct, or NULL. Release with
 *         message_status_free.
 */
MessageStatus* message_status_get(sqlite3* db, DB_ID id);




/**
 * Fetches every MessageStatus row, ordered by id.
 *
 * @param db        (sqlite3*, Required) — open database.
 * @param out_count (int*, Required) — receives the number of rows.
 * @return (MessageStatus*) — heap array, or NULL when empty. Release
 *         with message_status_free_all.
 */
MessageStatus* message_status_get_all(sqlite3* db, int* out_count);




/**
 * Replaces the name of an existing MessageStatus.
 *
 * @param db   (sqlite3*, Required) — open database.
 * @param id   (DB_ID, Required) — row id.
 * @param name (const char*, Required) — new name.
 * @return (int) — 0 on success; -1 if not found or on SQL error.
 */
int message_status_update(sqlite3* db, DB_ID id, const char* name);




/**
 * Deletes a MessageStatus row.
 *
 * @param db (sqlite3*, Required) — open database.
 * @param id (DB_ID, Required) — row id.
 * @return (int) — 0 on success; -1 on SQL error or if another row
 *         still references this id.
 */
int message_status_delete(sqlite3* db, DB_ID id);
