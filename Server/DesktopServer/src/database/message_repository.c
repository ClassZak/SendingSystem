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
 * CRUD implementation for the Message table.
 */

#include "message_repository.h"
#include "model/db_helpers.h"
#include <stdlib.h>



static const char* k_insert = "INSERT INTO Message(connectionId, messageStatusId, messageTypeId, directionId, response, sendingTime, request) VALUES(?,?,?,?,?,?,?)";
static const char* k_get    = "SELECT id, connectionId, messageStatusId, messageTypeId, directionId, response, sendingTime, request FROM Message WHERE id=?";
static const char* k_getall = "SELECT id, connectionId, messageStatusId, messageTypeId, directionId, response, sendingTime, request FROM Message ORDER BY id";
static const char* k_lastok = "SELECT id, connectionId, messageStatusId, messageTypeId, directionId, response, sendingTime, request FROM Message WHERE messageStatusId=2 ORDER BY id DESC LIMIT 1";
static const char* k_update = "UPDATE Message SET connectionId=?, messageStatusId=?, messageTypeId=?, directionId=?, response=?, sendingTime=?, request=? WHERE id=?";
static const char* k_delete = "DELETE FROM Message WHERE id=?";




static Message* row_to_message(sqlite3_stmt* s) {
	Message* m = malloc(sizeof(*m));

	if (!m) {
		return NULL;
	}

	m->id                = (DB_ID)sqlite3_column_int64(s, 0);
	m->connection_id     = (DB_ID)sqlite3_column_int64(s, 1);
	m->message_status_id = (DB_ID)sqlite3_column_int64(s, 2);
	m->message_type_id   = (DB_ID)sqlite3_column_int64(s, 3);
	m->direction_id      = (DB_ID)sqlite3_column_int64(s, 4);
	m->response          = db_dup_col(s, 5);
	m->sending_time      = db_dup_col(s, 6);
	m->request           = db_dup_col(s, 7);

	return m;
}




int message_insert(sqlite3* db, DB_ID connection_id, DB_ID status_id, DB_ID type_id, DB_ID direction_id,
                   const char* response, const char* sending_time,
                   const char* request, DB_ID* out_id) {
	sqlite3_stmt* s = NULL;

	if (sqlite3_prepare_v2(db, k_insert, -1, &s, NULL) != SQLITE_OK) {
		return -1;
	}

	sqlite3_bind_int64(s, 1, (sqlite3_int64)connection_id);
	sqlite3_bind_int64(s, 2, (sqlite3_int64)status_id);
	sqlite3_bind_int64(s, 3, (sqlite3_int64)type_id);
	sqlite3_bind_int64(s, 4, (sqlite3_int64)direction_id);
	db_bind_text_or_null(s, 5, response);
	db_bind_text_or_null(s, 6, sending_time);
	db_bind_text_or_null(s, 7, request);
	int rc = sqlite3_step(s);
	sqlite3_finalize(s);

	if (rc != SQLITE_DONE) {
		return -1;
	}

	if (out_id) {
		*out_id = (DB_ID)sqlite3_last_insert_rowid(db);
	}

	return 0;
}




Message* message_get(sqlite3* db, DB_ID id) {
	sqlite3_stmt* s = NULL;

	if (sqlite3_prepare_v2(db, k_get, -1, &s, NULL) != SQLITE_OK) {
		return NULL;
	}

	sqlite3_bind_int64(s, 1, (sqlite3_int64)id);

	Message* out = NULL;
	if (sqlite3_step(s) == SQLITE_ROW) {
		out = row_to_message(s);
	}
	sqlite3_finalize(s);

	return out;
}




Message* message_get_all(sqlite3* db, int* out_count) {
	*out_count = 0;
	sqlite3_stmt* s = NULL;

	if (sqlite3_prepare_v2(db, k_getall, -1, &s, NULL) != SQLITE_OK) {
		return NULL;
	}

	Message* arr = NULL;
	int count = 0, cap = 0;

	while (sqlite3_step(s) == SQLITE_ROW) {
		if (count >= cap) {
			cap = cap ? cap * 2 : 8;
			Message* t = realloc(arr, (size_t)cap * sizeof(*arr));
			if (!t) {
				message_free_all(arr, count);
				sqlite3_finalize(s);

				return NULL;
			}
			arr = t;
		}
		Message* row = row_to_message(s);
		if (!row) {
			message_free_all(arr, count);
			sqlite3_finalize(s);

			return NULL;
		}
		arr[count++] = *row;
		free(row);
	}

	sqlite3_finalize(s);
	*out_count = count;

	return arr;
}




Message* message_get_last_successful(sqlite3* db) {
	sqlite3_stmt* s = NULL;

	if (sqlite3_prepare_v2(db, k_lastok, -1, &s, NULL) != SQLITE_OK) {
		return NULL;
	}

	Message* out = NULL;
	if (sqlite3_step(s) == SQLITE_ROW) {
		out = row_to_message(s);
	}
	sqlite3_finalize(s);

	return out;
}




int message_update(sqlite3* db, DB_ID id, DB_ID connection_id, DB_ID status_id, DB_ID type_id, DB_ID direction_id,
                   const char* response, const char* sending_time, const char* request) {
	sqlite3_stmt* s = NULL;

	if (sqlite3_prepare_v2(db, k_update, -1, &s, NULL) != SQLITE_OK) {
		return -1;
	}

	sqlite3_bind_int64(s, 1, (sqlite3_int64)connection_id);
	sqlite3_bind_int64(s, 2, (sqlite3_int64)status_id);
	sqlite3_bind_int64(s, 3, (sqlite3_int64)type_id);
	sqlite3_bind_int64(s, 4, (sqlite3_int64)direction_id);
	db_bind_text_or_null(s, 5, response);
	db_bind_text_or_null(s, 6, sending_time);
	db_bind_text_or_null(s, 7, request);
	sqlite3_bind_int64(s, 8, (sqlite3_int64)id);
	int rc = sqlite3_step(s);
	sqlite3_finalize(s);

	return rc == SQLITE_DONE && sqlite3_changes(db) > 0 ? 0 : -1;
}




int message_delete(sqlite3* db, DB_ID id) {
	sqlite3_stmt* s = NULL;

	if (sqlite3_prepare_v2(db, k_delete, -1, &s, NULL) != SQLITE_OK) {
		return -1;
	}

	sqlite3_bind_int64(s, 1, (sqlite3_int64)id);
	int rc = sqlite3_step(s);
	sqlite3_finalize(s);

	return rc == SQLITE_DONE && sqlite3_changes(db) > 0 ? 0 : -1;
}

