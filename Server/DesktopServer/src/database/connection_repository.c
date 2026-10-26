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
 * CRUD implementation for the Connection table.
 */

#include "connection_repository.h"
#include "model/db_helpers.h"
#include <stdlib.h>




static const char* k_insert = "INSERT INTO Connection(ip, port, isSSLEncrypted, roleId) VALUES(?,?,?,?)";
static const char* k_get    = "SELECT id, ip, port, isSSLEncrypted, roleId FROM Connection WHERE id=?";
static const char* k_getall = "SELECT id, ip, port, isSSLEncrypted, roleId FROM Connection ORDER BY id";
static const char* k_find   = "SELECT id FROM Connection WHERE ip=? AND port=? AND isSSLEncrypted=? AND roleId=?";
static const char* k_update = "UPDATE Connection SET ip=?, port=?, isSSLEncrypted=?, roleId=? WHERE id=?";
static const char* k_delete = "DELETE FROM Connection WHERE id=?";



int connection_insert(sqlite3* db, const char* ip, int port, int ssl, DB_ID role_id, DB_ID* out_id) {
	sqlite3_stmt* s = NULL;

	if (sqlite3_prepare_v2(db, k_insert, -1, &s, NULL) != SQLITE_OK) {
		return -1;
	}

	db_bind_text_or_null(s, 1, ip);
	sqlite3_bind_int(s, 2, port);
	sqlite3_bind_int(s, 3, ssl);
	sqlite3_bind_int(s, 4, role_id);
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




Connection* connection_get(sqlite3* db, DB_ID id) {
	sqlite3_stmt* s = NULL;

	if (sqlite3_prepare_v2(db, k_get, -1, &s, NULL) != SQLITE_OK) {
		return NULL;
	}

	sqlite3_bind_int64(s, 1, (sqlite3_int64)id);
	Connection* out = NULL;

	if (sqlite3_step(s) == SQLITE_ROW) {
		out = malloc(sizeof(*out));
		if (out) {
			out->id               = (DB_ID)sqlite3_column_int64(s, 0);
			out->ip               = db_dup_col(s, 1);
			out->port             = sqlite3_column_int(s, 2);
			out->is_ssl_encrypted = sqlite3_column_int(s, 3);
			out->role_id          = sqlite3_column_int(s, 4);
		}
	}

	sqlite3_finalize(s);
	return out;
}




Connection* connection_get_all(sqlite3* db, int* out_count) {
	*out_count = 0;
	sqlite3_stmt* s = NULL;

	if (sqlite3_prepare_v2(db, k_getall, -1, &s, NULL) != SQLITE_OK) {
		return NULL;
	}

	Connection* arr = NULL;
	int count = 0, cap = 0;

	while (sqlite3_step(s) == SQLITE_ROW) {
		if (count >= cap) {
			cap = cap ? cap * 2 : 8;
			Connection* t = realloc(arr, (size_t)cap * sizeof(*arr));
			if (!t) {
				connection_free_all(arr, count);
				sqlite3_finalize(s);

				return NULL;
			}
			arr = t;
		}
		arr[count].id               = (DB_ID)sqlite3_column_int64(s, 0);
		arr[count].ip               = db_dup_col(s, 1);
		arr[count].port             = sqlite3_column_int(s, 2);
		arr[count].is_ssl_encrypted = sqlite3_column_int(s, 3);
		arr[count].role_id          = sqlite3_column_int(s, 4);
		++count;
	}

	sqlite3_finalize(s);
	*out_count = count;

	return arr;
}




DB_ID connection_get_or_create(sqlite3* db, const char* ip, int port, int ssl, DB_ID role_id) {
	sqlite3_stmt* s = NULL;

	if (sqlite3_prepare_v2(db, k_find, -1, &s, NULL) != SQLITE_OK) {
		return -1;
	}

	db_bind_text_or_null(s, 1, ip);
	sqlite3_bind_int(s, 2, port);
	sqlite3_bind_int(s, 3, ssl);
	sqlite3_bind_int(s, 4, role_id);

	DB_ID id = -1;
	if (sqlite3_step(s) == SQLITE_ROW) {
		id = (DB_ID)sqlite3_column_int64(s, 0);
	}
	sqlite3_finalize(s);

	if (id >= 0) {
		return id;
	}

	if (connection_insert(db, ip, port, ssl, role_id, &id) != 0) {
		return -1;
	}

	return id;
}




int connection_update(sqlite3* db, DB_ID id, const char* ip, int port, int ssl, DB_ID role_id) {
	sqlite3_stmt* s = NULL;

	if (sqlite3_prepare_v2(db, k_update, -1, &s, NULL) != SQLITE_OK) {
		return -1;
	}

	db_bind_text_or_null(s, 1, ip);
	sqlite3_bind_int(s, 2, port);
	sqlite3_bind_int(s, 3, ssl);
	sqlite3_bind_int(s, 4, role_id);
	sqlite3_bind_int64(s, 5, (sqlite3_int64)id);
	int rc = sqlite3_step(s);
	sqlite3_finalize(s);

	return rc == SQLITE_DONE && sqlite3_changes(db) > 0 ? 0 : -1;
}




int connection_delete(sqlite3* db, DB_ID id) {
	sqlite3_stmt* s = NULL;

	if (sqlite3_prepare_v2(db, k_delete, -1, &s, NULL) != SQLITE_OK) {
		return -1;
	}

	sqlite3_bind_int64(s, 1, (sqlite3_int64)id);
	int rc = sqlite3_step(s);
	sqlite3_finalize(s);

	return rc == SQLITE_DONE && sqlite3_changes(db) > 0 ? 0 : -1;
}

