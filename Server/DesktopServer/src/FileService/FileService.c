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
#include "FileService.h"
#include "../PrintProcedures/PrintProcedures.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <regex.h>
#ifdef _WIN32
#include <direct.h>
#define get_pwd _getcwd
#else
#include <unistd.h>
#include <errno.h>
#define get_pwd getcwd
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif




void sanitize_filename(char* dest, size_t dest_size, const char* src)
{
	if (!src)
	{
		strncpy(dest, "unnamed.dat", dest_size);
		return;
	}
	
	regex_t invalid_regex;
	if (regcomp(&invalid_regex, "[^[:alnum:]._-]", REG_EXTENDED))
	{
		strncpy(dest, "unnamed.dat", dest_size);
		return;
	}

	char temp[MAX_FILENAME + 1];
	size_t temp_len = 0;
	const char* p = src;
	regmatch_t match;
	while (*p && temp_len < MAX_FILENAME)
	{
		if (regexec(&invalid_regex, p, 1, &match, 0) == 0)
		{
			// Copy everything before illegal symbol
			size_t len = match.rm_so;
			if (temp_len + len >= MAX_FILENAME) len = MAX_FILENAME - temp_len - 1;
				strncpy(temp + temp_len, p, len);
			temp_len += len;
			if (temp_len < MAX_FILENAME) temp[temp_len++] = '_';
				p += match.rm_eo;
		}
		else
		{
			size_t remaining = strlen(p);
			if (temp_len + remaining >= MAX_FILENAME)
				remaining = MAX_FILENAME - temp_len - 1;
			strncpy(temp + temp_len, p, remaining);
			temp_len += remaining;
			break;
		}
	}
	temp[temp_len] = '\0';
	regfree(&invalid_regex);

	char* dotdot;
	while ((dotdot = strstr(temp, "..")) != NULL)
	{
		dotdot[0] = '_';
		dotdot[1] = '_';
		memmove(dotdot + 1, dotdot + 2, strlen(dotdot + 2) + 1);
	}
	char *final = temp;
	if (final[0] == '.')
		++final;

	if (strlen(final) == 0)
		strncpy(dest, "unnamed.dat", dest_size);
	else
		strncpy(dest, final, dest_size);

	dest[dest_size - 1] = '\0';
}




void print_pwd() {
	char buffer[0x1000]; // 4096
	if (get_pwd(buffer, sizeof(buffer))) {
		print_info("Current working directory:%s\n", buffer);
	} else {
		print_error("Error getting current directory\n");
	}
}




bool file_exists(const char* filename) {
	bool result = false;

#ifndef _WIN32
	return access(filename, 0) == 0;
#else
	return _access(filename, 0) == 0;
#endif
}




char* read_file_full(const char* filename, long* data_size) {
	if (!file_exists(filename)) {
		int err;
#ifndef _WIN32
		err = errno;
#else
		err = GetLastError();
#endif
		print_error("Error read file \'%s\': code %d %s\n", filename, err, strerror(err));
		return NULL;
	}


	FILE* file;
	if ((file = fopen(filename, "r")) == 0) {
		int err;
#ifndef _WIN32
		err = errno;
#else
		err = GetLastError();
#endif
		print_error("Error read database file (%d): %s\n", err, strerror(err));
	}


	if (fseek(file, 0, SEEK_END)) {
		fclose(file);
		return NULL;
	}


	*data_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	char* result = (char*)malloc((*data_size) + 1);
	if (!result) {
		print_error("Failed to allocate %d bytes\n", data_size);
		return NULL;
	}

	fread(result, sizeof(char), *data_size, file);
	fclose(file);

	return result;
}




wchar_t* read_file_full_utf8(const char* filename, long* data_size) {
	if (!file_exists(filename)) {
		int err;
#ifndef _WIN32
		err = errno;
#else
		err = GetLastError();
#endif
		print_error("Error read file \'%s\': code %d %s\n", filename, err, strerror(err));
		return NULL;
	}


	FILE* file;
	if ((file = fopen(filename, "r")) == 0) {
		int err;
#ifndef _WIN32
		err = errno;
#else
		err = GetLastError();
#endif
		print_error("Error read database file (%d): %s\n", err, strerror(err));
	}


	if (fseek(file, 0, SEEK_END)) {
		fclose(file);
		return NULL;
	}

	*data_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	wchar_t* result = (wchar_t*)malloc(((*data_size) + 1) * sizeof(wchar_t));
	if (!result) {
		print_error("Failed to allocate %d bytes\n", data_size);
		return NULL;
	}

	fread(result, sizeof(wchar_t), *data_size, file);
	fclose(file);

	return result;
}




int touch_file(const char *filename) {
#ifdef _WIN32
	// --- Windows Implementation (Win32 API) ---
	HANDLE hFile = CreateFileA(
		filename,
		FILE_WRITE_ATTRIBUTES, 
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS, // Creates if missing, opens if existing
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (hFile == INVALID_HANDLE_VALUE) {
		print_error("[Windows Error] Cannot open/create file. Code: %lu\n", GetLastError());
		return -1;
	}

	// Get current system time and map to FILETIME structure
	SYSTEMTIME now_sys;
	FILETIME now_file;
	GetSystemTime(&now_sys);
	SystemTimeToFileTime(&now_sys, &now_file);

	// Apply to Last Access and Last Write parameters
	if (!SetFileTime(hFile, NULL, &now_file, &now_file)) {
		fprintf("[Windows Error] Cannot update timestamp. Code: %lu\n", GetLastError());
		CloseHandle(hFile);
		return -1;
	}

	CloseHandle(hFile);
	return 0;

#else
	// --- Linux / POSIX Implementation (Standard C + modern POSIX) ---
	// Open in append mode to create safely without destroying existing file text
	FILE *file = fopen(filename, "a");
	if (file == NULL) {
		print_error("[Linux Error] File access failed\n");
		return -1;
	}
	fclose(file);

	// Update Access and Modification flags to present time (Passing NULL forces current time)
	if (utimensat(AT_FDCWD, filename, NULL, 0) < 0) {
		print_error("[Linux Error] Timestamp update failed\n");
		return -1;
	}

	return 0;
#endif
}

