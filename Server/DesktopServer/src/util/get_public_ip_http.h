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
 * @file    get_public_ip_http.h
 * @brief   Obtains the public IP address by querying an external
 *          HTTPS service.
 *
 * Requires libcurl (-lcurl). Unlike the CivetWeb client, libcurl
 * handles DNS, TLS, redirects and timeouts out of the box, so this is
 * the fallback when UPnP is unavailable.
 */

#pragma once

/**
 * @brief Queries https://api.ipify.org for the caller's public IP.
 *
 * @return Heap-allocated NUL-terminated string on success, NULL on
 *         network error, timeout (5 s), or malformed response.
 *
 * @note Release the result with free().
 */
char* get_public_ip_http(void);
