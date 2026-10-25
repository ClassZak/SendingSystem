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
#include "api_send.h"
#include "http_json.h"
#include "../util/config.h"
#include "../FileService/FileService.h"
#include "../Base64/Base64.h"
#include "../PrintProcedures/PrintProcedures.h"

#include <cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

extern pthread_mutex_t mtx_print;
extern pthread_mutex_t mtx_fprint;

/* Saves data to SAVE_DIR/<safe_name>.
 * type == "FILE" → base64-decode, otherwise writes as-is.
 * Returns 0 on success. */
static int save_to_file(const char* type, const char* data,
						const char* saveToPath, int* written_out) {
	char safe_name[MAX_FILENAME + 1];
	sanitize_filename(safe_name, sizeof(safe_name), saveToPath);

	pthread_mutex_lock(&mtx_fprint);
#ifdef _WIN32
	_mkdir(SAVE_DIR);
#else
	mkdir(SAVE_DIR, 0755);
#endif
	pthread_mutex_unlock(&mtx_fprint);

	char full_path[MAX_FILENAME + SAVE_DIR_LENGTH + 2];
	snprintf(full_path, sizeof(full_path), "%s/%s", SAVE_DIR, safe_name);

	pthread_mutex_lock(&mtx_fprint);
	FILE* f = fopen(full_path, "wb");
	if (!f) {
		pthread_mutex_unlock(&mtx_fprint);
		print_error("Cannot open %s\n", full_path);
		return -1;
	}

	int written = 0;
	if (strcmp(type, "FILE") == 0) {
		size_t b64_len = strlen(data);
		unsigned char* bin = malloc(b64_len);
		if (bin) {
			int dec = base64_decode(data, bin, (int)b64_len);
			if (dec > 0) { fwrite(bin, 1, dec, f); written = dec; }
			free(bin);
		}
	} else {
		written = (int)strlen(data);
		fputs(data, f);
	}
	fclose(f);
	pthread_mutex_unlock(&mtx_fprint);

	print_info("[%s] saved to %s (%d bytes)\n", type, full_path, written);
	if (written_out)
		*written_out = written;

	return 0;
}




/*
 * Handle /api/send request
 * what receives text client data
 *
 * @param conn civetweb connection
 * @param cbdata received data
 *
 * POST method
 *
 * JSON body:
 *	<type> ::= string <RAW_DATA|FILE>
 *	<data> ::= string
 */
int api_send_handler(struct mg_connection* conn, void* cbdata) {
	const struct mg_request_info* ri = mg_get_request_info(conn);

	if (strcmp(ri->request_method, "POST") != 0) {
		http_send_error(conn, 405, "POST required");
		return 1;
	}

	long long cl = 0;
	const char* cls = mg_get_header(conn, "Content-Length");
	if (cls) cl = atoll(cls);
	if (cl <= 0 || cl > MAX_MESSAGE_SIZE) {
		http_send_error(conn, 400, "Invalid Content-Length");
		return 1;
	}	

	char* body = malloc((size_t)cl + 1);
	if (!body) { http_send_error(conn, 500, "Memory error"); return 1; }
	int n = mg_read(conn, body, (size_t)cl);
	if (n != cl) {
		free(body);
		http_send_error(conn, 400, "Incomplete body");
		return 1;
	}
	body[cl] = '\0';

	cJSON* root = cJSON_Parse(body);
	free(body);
	if (!root) {
		http_send_error(conn, 400, "JSON parse error");
		return 1;
	}

	cJSON* typeItem   = cJSON_GetObjectItem(root, "type");
	cJSON* dataItem   = cJSON_GetObjectItem(root, "data");
	cJSON* saveToPath = cJSON_GetObjectItem(root, "saveToPath");

	if (!cJSON_IsString(typeItem) || !cJSON_IsString(dataItem)) {
		cJSON_Delete(root);
		http_send_error(conn, 400, "Missing 'type' or 'data'");
		return 1;
	}

	const char* type = typeItem->valuestring;
	const char* data = dataItem->valuestring;
	bool need_save = cJSON_IsString(saveToPath) && saveToPath->valuestring[0] != '\0';

	if (!need_save) {
		pthread_mutex_lock(&mtx_print);
		printf("[%s]: %s\n", type, data);
		pthread_mutex_unlock(&mtx_print);
		http_send_json(conn, 200, "message", "OK");
	} else if (save_to_file(type, data, saveToPath->valuestring, NULL) == 0) {
		http_send_json(conn, 200, "message", "File saved");
	} else {
		http_send_error(conn, 500, "Cannot save file");
	}

	cJSON_Delete(root);
	return 1;
}

