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
 * File system helpers — see FileService.c.
 *
 * Path limits live in util/config.h; this header only pulls them in.
 */
#pragma once

#include "../util/config.h"


#include <stddef.h>
#include <stdbool.h>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif




/**
 * Sanitises a user-supplied filename to [A-Za-z0-9._-] only.
 *
 * @param dest      (char*, Required) — output buffer.
 * @param dest_size (size_t, Required) — capacity of dest.
 * @param src       (const char*, Optional) — source name.
 */
void sanitize_filename(char* dest, size_t dest_size, const char* src);




/**
 * Logs the current working directory.
 */
void print_pwd(void);




/**
 * Checks whether a file exists at the given path.
 *
 * @param filename (const char*, Required) — path to test.
 * @return (bool) — true if the path exists.
 */
bool file_exists(const char* filename);




/**
 * Reads an entire file into a heap buffer (NUL-terminated).
 *
 * @param filename  (const char*, Required) — file path.
 * @param data_size (long*, Required) — receives the byte count.
 * @return (char*) — heap buffer, or NULL on error. Caller frees.
 */
char* read_file_full(const char* filename, long* data_size);




/**
 * Creates an empty file or refreshes its timestamps.
 *
 * @param filename (const char*, Required) — file path.
 * @return (int) — 0 on success; -1 on error.
 */
int touch_file(const char* filename);
