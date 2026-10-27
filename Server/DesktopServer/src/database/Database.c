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
 * Opens the SQLite database and applies the schema.
 */

#include "Database.h"
#include "database_config.h"
#include "../FileService/FileService.h"
#include "../PrintProcedures/PrintProcedures.h"


#include <stdlib.h>




/**
 * Opens (or creates) the database and applies the schema.
 *
 * @param filename (const char*, Required) — SQLite file path.
 * @param out_db   (sqlite3**, Required) — receives the open handle.
 * @return (int) — 0 on success; -1 on any failure.
 *
 * @note Caller owns the handle; release with database_close.
 */
int database_open(const char* filename, sqlite3** out_db) {
	sqlite3* db = NULL;

	if (sqlite3_open(filename, &db) != SQLITE_OK) {
		print_error("Cannot open %s: %s\n", filename, sqlite3_errmsg(db));
		print_error("Error in file '%s', line:%d in '%s'\n", __FILE__, __LINE__, __FUNCTION__);
		sqlite3_close(db);

		return -1;
	}

	sqlite3_exec(db, "PRAGMA foreign_keys = ON", NULL, NULL, NULL);
	sqlite3_exec(db, "PRAGMA journal_mode = WAL", NULL, NULL, NULL);


	long size = 0;
	char* sql = read_file_full(CREATE_DATABASE_FILE, &size);
	if (!sql) {
		print_error("Cannot read %s\n", CREATE_DATABASE_FILE);
		print_error("Error in file '%s', line:%d in '%s'\n", __FILE__, __LINE__, __FUNCTION__);
		sqlite3_close(db);

		return -1;
	}


	char* err = NULL;
	int rc = sqlite3_exec(db, sql, NULL, NULL, &err);
	free(sql);

	if (rc != SQLITE_OK) {
		print_error("Schema: %s\n", err ? err : "unknown");
		print_error("Error in file '%s', line:%d in '%s'\n", __FILE__, __LINE__, __FUNCTION__);
		sqlite3_free(err);
		sqlite3_close(db);

		return -1;
	}

	print_success("Database ready: %s\n", filename);
	*out_db = db;

	return 0;
}




/**
 * Closes a handle returned by database_open.
 *
 * @param db (sqlite3*, Optional) — handle; NULL is safe.
 */
void database_close(sqlite3* db) {
	if (db) {
		sqlite3_close(db);
	}
}

