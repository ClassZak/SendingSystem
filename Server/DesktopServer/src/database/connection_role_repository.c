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
 * CRUD implementation for the ConnectionRole table.
 */

#include "connection_role_repository.h"
#include "model/db_helpers.h"
#include <stdlib.h>




static const char* k_insert = "INSERT INTO ConnectionRole(name) VALUES(?)";
static const char* k_get    = "SELECT id, name FROM ConnectionRole WHERE id=?";
static const char* k_getall = "SELECT id, name FROM ConnectionRole ORDER BY id";
static const char* k_update = "UPDATE ConnectionRole SET name=? WHERE id=?";
static const char* k_delete = "DELETE FROM ConnectionRole WHERE id=?";




int connection_role_insert(sqlite3* db, const char* name, DB_ID* out_id) {
	sqlite3_stmt* s = NULL;

	if (sqlite3_prepare_v2(db, k_insert, -1, &s, NULL) != SQLITE_OK) {
		return -1;
	}

	sqlite3_bind_text(s, 1, name, -1, SQLITE_TRANSIENT);
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




ConnectionRole* connection_role_get(sqlite3* db, DB_ID id) {
	sqlite3_stmt* s = NULL;

	if (sqlite3_prepare_v2(db, k_get, -1, &s, NULL) != SQLITE_OK) {
		return NULL;
	}

	sqlite3_bind_int64(s, 1, (sqlite3_int64)id);
	ConnectionRole* out = NULL;

	if (sqlite3_step(s) == SQLITE_ROW) {
		out = malloc(sizeof(*out));
		if (out) {
			out->id   = (DB_ID)sqlite3_column_int64(s, 0);
			out->name = db_dup_col(s, 1);
		}
	}

	sqlite3_finalize(s);
	return out;
}




ConnectionRole* connection_role_get_all(sqlite3* db, int* out_count) {
	*out_count = 0;
	sqlite3_stmt* s = NULL;

	if (sqlite3_prepare_v2(db, k_getall, -1, &s, NULL) != SQLITE_OK) {
		return NULL;
	}

	ConnectionRole* arr = NULL;
	int count = 0, cap = 0;

	while (sqlite3_step(s) == SQLITE_ROW) {
		if (count >= cap) {
			cap = cap ? cap * 2 : 8;
			ConnectionRole* t = realloc(arr, (size_t)cap * sizeof(*arr));
			if (!t) {
				connection_role_free_all(arr, count);
				sqlite3_finalize(s);

				return NULL;
			}
			arr = t;
		}
		arr[count].id   = (DB_ID)sqlite3_column_int64(s, 0);
		arr[count].name = db_dup_col(s, 1);
		count++;
	}

	sqlite3_finalize(s);
	*out_count = count;

	return arr;
}




int connection_role_update(sqlite3* db, DB_ID id, const char* name) {
	sqlite3_stmt* s = NULL;

	if (sqlite3_prepare_v2(db, k_update, -1, &s, NULL) != SQLITE_OK) {
		return -1;
	}

	sqlite3_bind_text(s, 1, name, -1, SQLITE_TRANSIENT);
	sqlite3_bind_int64(s, 2, (sqlite3_int64)id);
	int rc = sqlite3_step(s);
	sqlite3_finalize(s);

	return rc == SQLITE_DONE && sqlite3_changes(db) > 0 ? 0 : -1;
}




int connection_role_delete(sqlite3* db, DB_ID id) {
	sqlite3_stmt* s = NULL;

	if (sqlite3_prepare_v2(db, k_delete, -1, &s, NULL) != SQLITE_OK) {
		return -1;
	}

	sqlite3_bind_int64(s, 1, (sqlite3_int64)id);
	int rc = sqlite3_step(s);
	sqlite3_finalize(s);

	return rc == SQLITE_DONE && sqlite3_changes(db) > 0 ? 0 : -1;
}

