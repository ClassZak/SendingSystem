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
    #define get_pwd getcwd
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

