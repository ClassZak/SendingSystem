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
 * File system helpers: filename sanitisation, existence checks,
 * full-file reads, touch, and current-directory logging.
 *
 * Path limits (SAVE_DIR, MAX_FILENAME, SAVE_DIR_LENGTH) live in
 * util/config.h — this module only consumes them.
 */

#include "FileService.h"
#include "../PrintProcedures/PrintProcedures.h"


#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <regex.h>
#include <errno.h>
#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define get_pwd _getcwd
#else
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#define get_pwd getcwd
#endif




/**
 * Sanitises a user-supplied filename to [A-Za-z0-9._-] only.
 *
 * Collapses `..` runs and strips a leading dot; NULL or empty input
 * becomes "unnamed.dat".
 *
 * @param dest      (char*, Required) — output buffer.
 * @param dest_size (size_t, Required) — capacity of dest.
 * @param src       (const char*, Optional) — source name.
 */
void sanitize_filename(char* dest, size_t dest_size, const char* src) {
	if (!src) {
		strncpy(dest, "unnamed.dat", dest_size);
		return;
	}

	regex_t invalid_regex;
	if (regcomp(&invalid_regex, "[^[:alnum:]._-]", REG_EXTENDED)) {
		strncpy(dest, "unnamed.dat", dest_size);
		return;
	}

	char temp[MAX_FILENAME + 1];
	size_t temp_len = 0;
	const char* p = src;
	regmatch_t match;

	while (*p && temp_len < MAX_FILENAME) {
		if (regexec(&invalid_regex, p, 1, &match, 0) == 0) {
			size_t len = match.rm_so;
			if (temp_len + len >= MAX_FILENAME) {
				len = MAX_FILENAME - temp_len - 1;
			}
			strncpy(temp + temp_len, p, len);
			temp_len += len;
			if (temp_len < MAX_FILENAME) {
				temp[temp_len++] = '_';
			}
			p += match.rm_eo;
		} else {
			size_t remaining = strlen(p);
			if (temp_len + remaining >= MAX_FILENAME) {
				remaining = MAX_FILENAME - temp_len - 1;
			}
			strncpy(temp + temp_len, p, remaining);
			temp_len += remaining;
			break;
		}
	}

	temp[temp_len] = '\0';
	regfree(&invalid_regex);


	char* dotdot;
	while ((dotdot = strstr(temp, "..")) != NULL) {
		dotdot[0] = '_';
		dotdot[1] = '_';
		memmove(dotdot + 1, dotdot + 2, strlen(dotdot + 2) + 1);
	}

	char* final = temp;
	if (final[0] == '.') {
		++final;
	}

	if (strlen(final) == 0) {
		strncpy(dest, "unnamed.dat", dest_size);
	} else {
		strncpy(dest, final, dest_size);
	}

	dest[dest_size - 1] = '\0';
}




/**
 * Logs the current working directory.
 *
 * Prints via print_info on success, print_error on failure.
 */
void print_pwd(void) {
	char buffer[0x1000];

	if (get_pwd(buffer, sizeof(buffer))) {
		print_info("Current working directory: %s\n", buffer);
	} else {
		print_error("Error getting current directory\n");
		print_error("Error in file '%s', line:%d in '%s'\n", __FILE__, __LINE__, __FUNCTION__);
	}
}




/**
 * Checks whether a file exists at the given path.
 *
 * @param filename (const char*, Required) — path to test.
 * @return (bool) — true if the path exists.
 */
bool file_exists(const char* filename) {
#ifndef _WIN32
	return access(filename, 0) == 0;
#else
	return _access(filename, 0) == 0;
#endif
}




/**
 * Reads an entire file into a heap buffer.
 *
 * The returned buffer is NUL-terminated, so it is safe to pass to
 * string-expecting APIs such as sqlite3_exec.
 *
 * @param filename  (const char*, Required) — file path.
 * @param data_size (long*, Required) — receives the byte count.
 * @return (char*) — heap buffer, or NULL on error. Caller frees.
 */
char* read_file_full(const char* filename, long* data_size) {
	if (!file_exists(filename)) {
		unsigned long err;
#ifdef _WIN32
    err = GetLastError();
    print_error("Error reading file '%s': code %lu\n", filename, (unsigned long)err);
	print_error("Error in file '%s', line:%d in '%s'\n", __FILE__, __LINE__, __FUNCTION__);
#else
    err = errno;
    print_error("Error reading file '%s': code %d %s\n",
                filename, err, strerror(err));
	print_error("Error in file '%s', line:%d in '%s'\n", __FILE__, __LINE__, __FUNCTION__);
#endif
		return NULL;
	}

	FILE* file = fopen(filename, "rb");
	if (!file) {
		unsigned long err;
#ifdef _WIN32
		err = GetLastError();
		print_error("Error reading file '%s': code %lu\n", filename, (unsigned long)err);
		print_error("Error in file '%s', line:%d in '%s'\n", __FILE__, __LINE__, __FUNCTION__);
#else
		err = errno;
		print_error("Error reading file '%s': code %d %s\n",
					filename, err, strerror(err));
		print_error("Error in file '%s', line:%d in '%s'\n", __FILE__, __LINE__, __FUNCTION__);
#endif
		return NULL;
	}

	if (fseek(file, 0, SEEK_END)) {
		fclose(file);
		return NULL;
	}

	*data_size = ftell(file);
	if (*data_size < 0) {
		fclose(file);
		return NULL;
	}
	fseek(file, 0, SEEK_SET);


	char* result = malloc((size_t)*data_size + 1);
	if (!result) {
		fclose(file);
		return NULL;
	}

	size_t read = fread(result, 1, (size_t)*data_size, file);
	result[read] = '\0';
	*data_size = (long)read;
	fclose(file);

	return result;
}




/**
 * Creates an empty file if it does not exist, or refreshes its
 * modification and access timestamps.
 *
 * @param filename (const char*, Required) — file path.
 * @return (int) — 0 on success; -1 on error.
 */
int touch_file(const char* filename) {
#ifdef _WIN32
	HANDLE hFile = CreateFileA(
		filename,
		FILE_WRITE_ATTRIBUTES,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (hFile == INVALID_HANDLE_VALUE) {
		print_error("Cannot open/create '%s': code %lu\n",
		            filename, GetLastError());
		print_error("Error in file '%s', line:%d in '%s'\n", __FILE__, __LINE__, __FUNCTION__);
		return -1;
	}

	SYSTEMTIME now_sys;
	FILETIME now_file;
	GetSystemTime(&now_sys);
	SystemTimeToFileTime(&now_sys, &now_file);

	if (!SetFileTime(hFile, NULL, &now_file, &now_file)) {
		print_error("Cannot update timestamp '%s': code %lu\n",
		            filename, GetLastError());
		print_error("Error in file '%s', line:%d in '%s'\n", __FILE__, __LINE__, __FUNCTION__);
		CloseHandle(hFile);
		return -1;
	}

	CloseHandle(hFile);
	return 0;
#else
	FILE* file = fopen(filename, "a");
	if (!file) {
		print_error("Cannot open/create '%s'\n", filename);
		print_error("Error in file '%s', line:%d in '%s'\n", __FILE__, __LINE__, __FUNCTION__);
		return -1;
	}
	fclose(file);

	if (utimensat(AT_FDCWD, filename, NULL, 0) < 0) {
		print_error("Cannot update timestamp '%s'\n", filename);
		print_error("Error in file '%s', line:%d in '%s'\n", __FILE__, __LINE__, __FUNCTION__);
		return -1;
	}

	return 0;
#endif
}

