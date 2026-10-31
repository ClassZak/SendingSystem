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
 * POST /api/send — accept text messages (JSON).
 *
 * Message type is determined by the endpoint, not by a "type" field:
 * /api/send is text-only, files go through /api/upload.
 */

#include "api_send.h"
#include "http_json.h"
#include "save_connection.h"
#include "save_message.h"
#include "../util/config.h"
#include "../util/now_timestamp.h"
#include "../FileService/FileService.h"
#include "../PrintProcedures/PrintProcedures.h"


#include <cJSON.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif




extern pthread_mutex_t mtx_print;
extern pthread_mutex_t mtx_fprint;




/**
 * Saves text data to SAVE_DIR/<sanitized_name>.
 *
 * Creates SAVE_DIR if it does not exist. The filename is passed
 * through sanitize_filename, so traversal attempts like "../etc/passwd"
 * collapse to a safe basename.
 *
 * @param data         (const char*, Required) — text to write.
 * @param save_to_path (const char*, Required) — user-supplied name;
 *                     sanitised before opening the file.
 * @param written_out  (int*, Optional) — receives byte count when set;
 *                     untouched on error.
 * @return (int) — 0 on success; -1 if the file cannot be opened.
 */
static int save_text_to_file(const char* data, const char* save_to_path,
                             int* written_out) {
	char safe_name[MAX_FILENAME + 1];
	sanitize_filename(safe_name, sizeof(safe_name), save_to_path);

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
		print_error("Error in file '%s', line:%d in '%s'\n",
		            __FILE__, __LINE__, __FUNCTION__);
		return -1;
	}

	int written = (int)strlen(data);
	fputs(data, f);
	fclose(f);
	pthread_mutex_unlock(&mtx_fprint);

	print_info("[RAW_DATA] saved to %s (%d bytes)\n", full_path, written);

	if (written_out) {
		*written_out = written;
	}

	return 0;
}




/**
 * Handles POST /api/send.
 *
 * Accepts JSON text data. If saveToPath is set, saves to
 * SAVE_DIR/<sanitized>; otherwise logs it. Regardless of the branch,
 * the (request, response) pair is written into the Message table so
 * the server keeps the same history the Android client keeps.
 *
 * **Method:** POST (other methods → 405).
 *
 * **Request body (application/json):**
 * ```
 * {
 *   "data":       string,   // Required. Text to send.
 *   "saveToPath": string    // Optional. Save to file when set.
 * }
 * ```
 *
 * **Responses:**
 *   - 200 OK — `{"type":"message","data":"OK"}` or
 *              `{"type":"message","data":"File saved"}`.
 *   - 400 Bad Request — bad Content-Length, incomplete body, bad JSON,
 *                        missing "data".
 *   - 405 Method Not Allowed — method is not POST.
 *   - 500 Internal Server Error — memory or file write failure.
 *
 * @param connection    (struct mg_connection*, Required) — CivetWeb connection.
 * @param callback_data (void*, Optional) — unused.
 * @return (int) — always 1.
 */
int api_send_handler(struct mg_connection* connection, void* callback_data) {
	const struct mg_request_info* request_info =
		mg_get_request_info(connection);

	/* Validate method before touching the database. */
	if (strcmp(request_info->request_method, "POST") != 0) {
		http_send_error(connection, 405, "POST required");
		return 1;
	}

	DB_ID connection_id = save_connection(connection);


	long long content_length = 0;
	const char* content_length_header =
		mg_get_header(connection, "Content-Length");
	if (content_length_header) {
		content_length = atoll(content_length_header);
	}

	if (content_length <= 0 || content_length > MAX_MESSAGE_SIZE) {
		http_send_error(connection, 400, "Invalid Content-Length");
		return 1;
	}


	char* body = malloc((size_t)content_length + 1);
	if (!body) {
		http_send_error(connection, 500, "Memory error");
		return 1;
	}

	long long bytes_read = mg_read(connection, body, (size_t)content_length);
	if (bytes_read != content_length) {
		free(body);
		http_send_error(connection, 400, "Incomplete body");
		return 1;
	}
	body[content_length] = '\0';


	cJSON* root = cJSON_Parse(body);
	if (!root) {
		free(body);
		http_send_error(connection, 400, "JSON parse error");
		return 1;
	}

	cJSON* data_item    = cJSON_GetObjectItem(root, "data");
	cJSON* save_to_item = cJSON_GetObjectItem(root, "saveToPath");

	if (!cJSON_IsString(data_item)) {
		cJSON_Delete(root);
		free(body);
		http_send_error(connection, 400, "Missing 'data'");
		return 1;
	}

	const char* data = data_item->valuestring;
	bool need_save = cJSON_IsString(save_to_item) &&
	                 save_to_item->valuestring[0] != '\0';


	/* Build the response once — reused for the DB row and the wire. */
	char* response_json = NULL;
	int   response_status = 200;

	if (!need_save) {
		pthread_mutex_lock(&mtx_print);
		print_info("[RAW_DATA]: %s\n", data);
		pthread_mutex_unlock(&mtx_print);

		response_json = http_build_json("message", "OK");
	} else if (save_text_to_file(data, save_to_item->valuestring, NULL) == 0) {
		response_json = http_build_json("message", "File saved");
	} else {
		response_status = 500;
		response_json = http_build_json("error", "Cannot save file");
	}


	/* Persist request+response before the client sees them. */
	if (connection_id != -1) {
		char sending_time[32];
		now_timestamp(sending_time, sizeof(sending_time));

		Message message;
		memset(&message, 0, sizeof(message));
		message.connection_id     = connection_id;
		message.message_status_id = (response_status == 200)
		                                ? MESSAGE_STATUS_SUCCESS
		                                : MESSAGE_STATUS_FAILED;
		message.message_type_id   = MESSAGE_TYPE_RAW_DATA;
		message.direction_id      = MESSAGE_DIRECTION_INBOUND;
		message.request           = body;
		message.response          = response_json;
		message.sending_time      = sending_time;

		save_message(&message);
	}


	http_send_built_json(connection, response_status, response_json);

	free(response_json);
	free(body);
	cJSON_Delete(root);

	return 1;
}

