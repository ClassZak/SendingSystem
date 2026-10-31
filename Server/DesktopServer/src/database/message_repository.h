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
 * SQLite CRUD for the Message table.
 */
#pragma once

#include "database_config.h"
#include "model/database_models.h"
#include <sqlite3.h>




/**
 * Inserts a new Message row.
 *
 * @param db            (sqlite3*, Required) — open database.
 * @param connection_id (DB_ID, Required) — FK to Connection.id.
 * @param status_id     (DB_ID, Required) — FK to MessageStatus.id.
 * @param type_id       (DB_ID, Required) — FK to MessageType.id.
 * @param direction_id  (DB_ID, Required) Message direction:
 *	1 outbound
 *	2 inbound
 * @param response      (const char*, Optional) — new response body.
 * @param sending_time  (const char*, Optional) — timestamp string.
 * @param request       (const char*, Optional) — client request body.
 * @param out_id        (DB_ID*, Optional) — receives the new row id.
 * @return (int) — 0 on success; -1 on SQL or FK error.
 */
int message_insert(sqlite3* db, DB_ID connection_id, DB_ID status_id, DB_ID type_id, DB_ID direction_id,
                   const char* response, const char* sending_time,
                   const char* request, DB_ID* out_id);




/**
 * Fetches a single Message by id.
 *
 * @param db (sqlite3*, Required) — open database.
 * @param id (DB_ID, Required) — row id.
 * @return (Message*) — heap struct, or NULL. Release with
 *         message_free.
 */
Message* message_get(sqlite3* db, DB_ID id);




/**
 * Fetches every Message row, ordered by id.
 *
 * @param db        (sqlite3*, Required) — open database.
 * @param out_count (int*, Required) — receives the number of rows.
 * @return (Message*) — heap array, or NULL when empty. Release with
 *         message_free_all.
 */
Message* message_get_all(sqlite3* db, int* out_count);




/**
 * Returns the most recent Message with status SUCCESS.
 *
 * Used by the UI to pre-fill the last working server endpoint.
 *
 * @param db (sqlite3*, Required) — open database.
 * @return (Message*) — heap struct, or NULL if no row / on error.
 *         Release with message_free.
 */
Message* message_get_last_successful(sqlite3* db);




/**
 * Replaces every mutable field of an existing Message.
 *
 * @param db            (sqlite3*, Required) — open database.
 * @param id            (DB_ID, Required) — row id.
 * @param connection_id (DB_ID, Required) — new FK to Connection.id.
 * @param status_id     (DB_ID, Required) — new FK to MessageStatus.id.
 * @param type_id       (DB_ID, Required) — new FK to MessageType.id.
 * @param direction_id  (DB_ID, Required) Message direction:
 *	1 outbound
 *	2 inbound
 * @param response      (const char*, Optional) — new response body.
 * @param sending_time  (const char*, Optional) — new timestamp.
 * @param request       (const char*, Optional) — new request body.
 * @return (int) — 0 on success; -1 if not found or on SQL/FK error.
 */
int message_update(sqlite3* db, DB_ID id, DB_ID connection_id, DB_ID status_id, DB_ID type_id, DB_ID direction_id,
                   const char* response, const char* sending_time, const char* request);




/**
 * Deletes a Message row.
 *
 * @param db (sqlite3*, Required) — open database.
 * @param id (DB_ID, Required) — row id.
 * @return (int) — 0 on success; -1 if not found or on SQL error.
 */
int message_delete(sqlite3* db, DB_ID id);
