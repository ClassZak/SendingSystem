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
 * Compile-time configuration for the Sending System server.
 *
 * All tunables live here. Variables are `static const` so each
 * translation unit gets its own private copy — this avoids the
 * classic "multiple definition" link error.
 */
#pragma once

#include <stdbool.h>




/* ---------- Network ---------- */
/** Listening port for HTTP and (optionally) HTTPS. */
#define CONNECTION_PORT  5000
/** Bind address; "0.0.0.0" accepts connections on all interfaces. */
static const char* CONNECTION_IP = "0.0.0.0";




/* ---------- TLS ---------- */
/** When true, CivetWeb opens the port in TLS mode. */
static const bool  SSL_ENCRYPT       = true;
/** PEM file with both certificate and key. */
static const char* PUBLIC_KEY_FILE   = "server.pem";
/** Legacy; not read by CivetWeb (key is inside PUBLIC_KEY_FILE). */
static const char* PRIVATE_KEY_FILE  = "server.key";




/* ---------- Limits ---------- */
/** Maximum body size for POST /api/send (100 MB). */
#define MAX_MESSAGE_SIZE   0x6400000
/** Per-file soft limit for POST /api/upload (1 GB). */
#define MAX_UPLOAD_SIZE    0x64000000
/** Idle-request timeout passed to CivetWeb. */
#define REQUEST_TIMEOUT_MS 30000




/* ---------- Files ---------- */
/** Root directory for received files. */
#define SAVE_DIR        "build/saves"
/** Maximum filename length in bytes. */
#define MAX_FILENAME    255
/** String length of SAVE_DIR (for fixed-size path buffers). */
#define SAVE_DIR_LENGTH 11




/* ---------- Messages ---------- */
/** Emitted when signal handlers cannot be installed. */
#define FAILED_SIGNAL_HANDLERS_MESSAGE "Failed to set up signal handlers\n"




#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <Windows.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <sys/select.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#endif







#ifndef _WIN32
typedef int SOCKET;
#define closesocket(s) close(s)
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR   (-1)
#endif
