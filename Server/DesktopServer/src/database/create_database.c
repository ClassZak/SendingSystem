#include "create_database.h"
#include "database_config.h"
#include "../PrintProcedures/PrintProcedures.h"
#include "../FileService/FileService.h"
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifndef _WIN32
#include <errno.h>
#endif




void create_database() {
	if (file_exists(DATABASE_FILENAME)) {
		print_info("Data base \'%s\' exists\n", DATABASE_FILENAME);
		return;
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
		return;
	}


	if (sqlite3_open(DATABASE_FILENAME, &db)) {
		print_error("Can not open data base: %s\n", sqlite3_errmsg(db));
		free(sql_create_prompt);
		return;
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
}

