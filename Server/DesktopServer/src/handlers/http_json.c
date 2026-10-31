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
 * Implementation of the JSON response helpers.
 *
 * All handlers go through http_send_json / http_send_error. Handlers
 * that need to persist the exact body they sent build it once with
 * http_build_json, hand the pointer to save_message, then push it out
 * with http_send_built_json — so the row in the database and the
 * bytes on the wire are always identical.
 */

#include "http_json.h"


#include <cJSON.h>


#include <string.h>
#include <stdlib.h>




/**
 * Builds the standard {"type":..., "data":...} JSON body.
 *
 * @param type (const char*, Required) — "message" or "error".
 * @param data (const char*, Required) — payload.
 * @return (char*) — heap string; caller frees.
 */
char* http_build_json(const char* type, const char* data) {
	cJSON* root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "type", type);
	cJSON_AddStringToObject(root, "data", data);

	char* json = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);

	return json;
}




/**
 * Sends an already-built JSON body with the given HTTP status and
 * closes the connection.
 *
 * @param conn   (struct mg_connection*, Required) — CivetWeb connection.
 * @param status (int, Required) — HTTP status code.
 * @param json   (const char*, Required) — NUL-terminated JSON body.
 */
void http_send_built_json(struct mg_connection* conn,
                          int status,
                          const char* json) {
	int length = json ? (int)strlen(json) : 0;

	const char* reason = (status == 200) ? "OK"
	                   : (status == 400) ? "Bad Request"
	                   : (status == 405) ? "Method Not Allowed"
	                   : (status == 500) ? "Internal Server Error"
	                   : "Error";

	mg_printf(conn,
	          "HTTP/1.1 %d %s\r\n"
	          "Content-Type: application/json\r\n"
	          "Content-Length: %d\r\n"
	          "Connection: close\r\n\r\n",
	          status, reason, length);
	if (length > 0) mg_write(conn, json, length);
}




/**
 * Sends `{"type": <type>, "data": <data>}` and closes the connection.
 *
 * @param conn   (struct mg_connection*, Required) — CivetWeb connection.
 * @param status (int, Required) — HTTP status code (200, 400, 405, 500…).
 * @param type   (const char*, Required) — value for the "type" field.
 * @param data   (const char*, Required) — value for the "data" field.
 */
void http_send_json(struct mg_connection* conn, int status,
                    const char* type, const char* data) {
	char* json = http_build_json(type, data);
	http_send_built_json(conn, status, json);
	free(json);
}




/**
 * Sends `{"type":"error","data":<msg>}` with the given HTTP status.
 *
 * @param conn   (struct mg_connection*, Required) — CivetWeb connection.
 * @param status (int, Required) — HTTP status code (4xx or 5xx).
 * @param msg    (const char*, Required) — error description.
 */
void http_send_error(struct mg_connection* conn, int status, const char* msg) {
	http_send_json(conn, status, "error", msg);
}

