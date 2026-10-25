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
 * JSON response helpers shared by all HTTP handlers.
 */
#pragma once

#include <civetweb.h>




/**
 * Sends `{"type": <type>, "data": <data>}` and closes the connection.
 *
 * @param conn   (struct mg_connection*, Required) — CivetWeb connection.
 * @param status (int, Required) — HTTP status code (200, 400, 405, 500…).
 * @param type   (const char*, Required) — value for the "type" field.
 * @param data   (const char*, Required) — value for the "data" field.
 */
void http_send_json(struct mg_connection* conn, int status,
                    const char* type, const char* data);




/**
 * Sends `{"type":"error","data":<msg>}` with the given HTTP status.
 *
 * @param conn   (struct mg_connection*, Required) — CivetWeb connection.
 * @param status (int, Required) — HTTP status code (4xx or 5xx).
 * @param msg    (const char*, Required) — error description.
 */
void http_send_error(struct mg_connection* conn, int status, const char* msg);
