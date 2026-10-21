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
#pragma once
#define SAVE_DIR "build/saves"
#define MAX_FILENAME 255
#define SAVE_DIR_LENGTH 11


#include <stddef.h>
#include <stdbool.h>
#ifdef _WIN32
#include <direct.h>   // _mkdir
#include <io.h>		  // _access
#else
#include <sys/stat.h> // mkdir
#include <unistd.h>   // access
#endif




#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#else
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif

void sanitize_filename(char* dest, size_t dest_size, const char* src);
void print_pwd();
bool file_exists(const char* filename);
char* read_file_full(const char* filename, long* data_size);
wchar_t* read_file_full_utf8(const char* filename, long* data_size);
int touch_file(const char *filename);
