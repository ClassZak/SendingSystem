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
 * POST /api/upload — stream multipart file uploads to disk.
 *
 * Each file part is written directly to SAVE_DIR/<sanitized_name>
 * without buffering the whole file in memory.
 */

#include "api_upload.h"
#include "http_json.h"
#include "../util/config.h"
#include "../FileService/FileService.h"
#include "../PrintProcedures/PrintProcedures.h"


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif




extern pthread_mutex_t mtx_print;
extern pthread_mutex_t mtx_fprint;




/** Per-request state shared between CivetWeb's form callbacks. */
typedef struct {
	FILE*     fp;                                        /**< Open file or NULL. */
	char      path[MAX_FILENAME + SAVE_DIR_LENGTH + 2];  /**< Current path. */
	long long size;                                      /**< Bytes written. */
} upload_ctx_t;




/**
 * Called by CivetWeb for each form field.
 *
 * For file parts (non-empty filename) opens the destination and
 * returns MG_FORM_FIELD_STORAGE_STORE. For plain fields returns
 * MG_FORM_FIELD_STORAGE_GET.
 *
 * @param key       (const char*, Required) — field name.
 * @param filename  (const char*, Required) — original filename.
 * @param path      (char*, Required) — destination buffer.
 * @param pathlen   (size_t, Required) — capacity of path.
 * @param user_data (void*, Required) — upload_ctx_t pointer.
 * @return (int) — one of MG_FORM_FIELD_STORAGE_*.
 */
static int on_field_found(const char* key, const char* filename,
                          char* path, size_t pathlen, void* user_data) {
	upload_ctx_t* u = user_data;

	if (!filename || !filename[0]) {
		return MG_FORM_FIELD_STORAGE_GET;
	}

	char safe_name[MAX_FILENAME + 1];
	sanitize_filename(safe_name, sizeof(safe_name), filename);

#ifdef _WIN32
	_mkdir(SAVE_DIR);
#else
	mkdir(SAVE_DIR, 0755);
#endif

	snprintf(path, pathlen, "%s/%s", SAVE_DIR, safe_name);
	snprintf(u->path, sizeof(u->path), "%s", path);
	u->size = 0;

	pthread_mutex_lock(&mtx_fprint);
	u->fp = fopen(path, "wb");
	pthread_mutex_unlock(&mtx_fprint);

	if (!u->fp) {
		print_error("Cannot open %s for write\n", path);
		print_error("Error in file '%s', line:%d in '%s'\n", __FILE__, __LINE__, __FUNCTION__);

		return MG_FORM_FIELD_STORAGE_ABORT;
	}

	print_info("[UPLOAD] Receiving %s\n", path);
	print_error("Error in file '%s', line:%d in '%s'\n", __FILE__, __LINE__, __FUNCTION__);

	return MG_FORM_FIELD_STORAGE_STORE;
}




/**
 * Called for plain (non-file) form fields. Currently ignored.
 *
 * @param key       (const char*, Required) — field name.
 * @param value     (const char*, Required) — field value.
 * @param valuelen  (size_t, Required) — value length.
 * @param user_data (void*, Required) — upload_ctx_t pointer.
 * @return (int) — always 0.
 */
static int on_field_get(const char* key, const char* value,
                        size_t valuelen, void* user_data) {
	return 0;
}




/**
 * Called after a file part is fully written.
 *
 * Closes the file, resets the state and logs the total size.
 *
 * @param path      (const char*, Required) — saved path.
 * @param size      (long long, Required) — file size.
 * @param user_data (void*, Required) — upload_ctx_t pointer.
 * @return (int) — always 0.
 */
static int on_field_store(const char* path, long long size, void* user_data) {
	upload_ctx_t* u = user_data;

	if (u->fp) {
		pthread_mutex_lock(&mtx_fprint);
		fclose(u->fp);
		pthread_mutex_unlock(&mtx_fprint);
		u->fp = NULL;
	}

	print_info("[UPLOAD] Stored %s (%lld bytes)\n", path, size);

	return 0;
}




/**
 * Handles POST /api/upload.
 *
 * @param conn   (struct mg_connection*, Required) — CivetWeb connection.
 * @param cbdata (void*, Optional) — unused.
 * @return (int) — always 1.
 */
int api_upload_handler(struct mg_connection* conn, void* cbdata) {
	const struct mg_request_info* ri = mg_get_request_info(conn);

	if (strcmp(ri->request_method, "POST") != 0) {
		http_send_error(conn, 405, "POST required");

		return 1;
	}


	upload_ctx_t u = {0};
	struct mg_form_data_handler fdh = {0};
	fdh.field_found = on_field_found;
	fdh.field_get   = on_field_get;
	fdh.field_store = on_field_store;
	fdh.user_data   = &u;

	int fields = mg_handle_form_request(conn, &fdh);

	if (u.fp) {
		fclose(u.fp);
		u.fp = NULL;
	}


	if (fields < 0) {
		http_send_error(conn, 400, "Form parse error");
	} else {
		http_send_json(conn, 200, "message", "File(s) received");
	}

	return 1;
}

