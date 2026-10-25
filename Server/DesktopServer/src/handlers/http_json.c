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
 */

#include "http_json.h"


#include <cJSON.h>


#include <string.h>
#include <stdlib.h>




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
	cJSON* root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "type", type);
	cJSON_AddStringToObject(root, "data", data);
	char* json = cJSON_PrintUnformatted(root);
	int len = json ? (int)strlen(json) : 0;

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
			  status, reason, len);
	if (len > 0) mg_write(conn, json, len);

	free(json);
	cJSON_Delete(root);
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
