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
 * @file config.h
 * @brief Compile-time configuration for the Sending System server.
 *
 * All tunables live here. Variables are `static const` so each
 * translation unit gets its own private copy — this avoids the classic
 * "multiple definition" link error and keeps the file header-only.
 */

#pragma once

#include <stdbool.h>




/* ---------- Network ---------- */

/** @brief Listening port for HTTP and (optionally) HTTPS. */
#define CONNECTION_PORT  5000

/** @brief Bind address; "0.0.0.0" accepts connections on all interfaces. */
static const char* CONNECTION_IP = "0.0.0.0";




/* ---------- TLS ---------- */

/** @brief When true, CivetWeb opens the port in TLS mode. */
static const bool  SSL_ENCRYPT       = true;

/** @brief Path to the PEM file containing both certificate and key. */
static const char* PUBLIC_KEY_FILE   = "server.pem";

/** @brief Legacy; not read by CivetWeb (key is inside PUBLIC_KEY_FILE). */
static const char* PRIVATE_KEY_FILE  = "server.key";




/* ---------- Limits ---------- */

/** @brief Maximum body size for POST /api/send (100 MB). */
#define MAX_MESSAGE_SIZE   0x6400000

/** @brief Per-file soft limit for POST /api/upload (1 GB). */
#define MAX_UPLOAD_SIZE    0x64000000

/** @brief Idle-request timeout passed to CivetWeb. */
#define REQUEST_TIMEOUT_MS 30000




/* ---------- Messages ---------- */

/** @brief Emitted when signal handlers cannot be installed. */
#define FAILED_SIGNAL_HANDLERS_MESSAGE "Failed to set up signal handlers\n"




#include <stdlib.h>
#include <string.h>
#include <stdio.h>

