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
 * POST /api/upload — multipart file upload endpoint.
 *
 * Streams each part to SAVE_DIR/<sanitized>; memory usage is
 * independent of file size.
 */
#pragma once

#include <civetweb.h>




/**
 * Handles POST /api/upload.
 *
 * @param conn   (struct mg_connection*, Required) — CivetWeb connection.
 * @param cbdata (void*, Optional) — unused.
 * @return (int) — always 1.
 */
int api_upload_handler(struct mg_connection* conn, void* cbdata);
