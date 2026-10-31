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
 * GET /api/openapi.json — OpenAPI 3.1 description of the public API.
 */
#pragma once

#include <civetweb.h>




/**
 * Handles GET /api/openapi.json — returns the OpenAPI document.
 *
 * @param connection    (struct mg_connection*, Required) — live connection.
 * @param callback_data (void*, Optional) — unused.
 * @return (int) — always 1.
 */
int api_openapi_handler(struct mg_connection* connection, void* callback_data);
