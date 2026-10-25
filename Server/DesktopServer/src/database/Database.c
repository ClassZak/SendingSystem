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
#include "Database.h"
#include "database_config.h"
#include "../FileService/FileService.h"
#include "../PrintProcedures/PrintProcedures.h"

#include <stdlib.h>




int create_database() {
	if (file_exists(DATABASE_FILENAME)) {
		print_info("Data base \'%s\' exists\n", DATABASE_FILENAME);
		return EXIT_FAILURE;
	} else {
		if (!touch_file(DATABASE_FILENAME)) {
			print_success("Data base file created \'%s\'\n", DATABASE_FILENAME);
		}
	}

	sqlite3* db;
	char* err_msq = 0;

	long sql_create_prompt_size;
	char* sql_create_prompt = read_file_full(CREATE_DATABASE_FILE, &sql_create_prompt_size);

	if (!sql_create_prompt) {
		print_error("Failed to read data base creation script\n");
		return EXIT_FAILURE;
	}


	if (sqlite3_open(DATABASE_FILENAME, &db)) {
		print_error("Can not open data base: %s\n", sqlite3_errmsg(db));
		free(sql_create_prompt);
		return EXIT_FAILURE;
	}


	int prompt_res;
	prompt_res = sqlite3_exec(db, sql_create_prompt, NULL, NULL, &err_msq);
	if (prompt_res == SQLITE_OK) {
		print_success("Data base was created sussecceful \'%s\'\n", DATABASE_FILENAME);
	} else {
		print_error("Failed to create data base: \'%s\'\n", err_msq);
		sqlite3_free(err_msq);
	}

	free(sql_create_prompt);

	return EXIT_SUCCESS;
}




int database_open(const char* filename, sqlite3** out_db) {
	sqlite3* db = NULL;

	if (sqlite3_open(filename, &db) != SQLITE_OK) {
		print_error("Cannot open %s: %s\n", filename, sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}

	sqlite3_exec(db, "PRAGMA foreign_keys = ON", NULL, NULL, NULL);
	sqlite3_exec(db, "PRAGMA journal_mode = WAL", NULL, NULL, NULL);

	long size = 0;
	char* sql = read_file_full(CREATE_DATABASE_FILE, &size);
	if (!sql) {
		print_error("Cannot read %s\n", CREATE_DATABASE_FILE);
		sqlite3_close(db);
		return -1;
	}

	char* err = NULL;
	int rc = sqlite3_exec(db, sql, NULL, NULL, &err);
	free(sql);

	if (rc != SQLITE_OK) {
		print_error("Schema: %s\n", err ? err : "unknown");
		sqlite3_free(err);
		sqlite3_close(db);
		return -1;
	}

	print_success("Database ready: %s\n", filename);
	*out_db = db;
	return 0;
}




void database_close(sqlite3* db) {
	if (db)
		sqlite3_close(db);
}

