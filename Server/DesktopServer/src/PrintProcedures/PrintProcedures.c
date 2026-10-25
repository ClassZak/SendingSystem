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
 * Coloured console output — implementation.
 *
 * This is the only module permitted to call printf/fprintf directly;
 * everything else goes through print_error/print_success/print_info.
 */

#include "PrintProcedures.h"


#ifndef _WIN32
#include <errno.h>
#include <string.h>
#endif




/**
 * Prints an error message to stderr in red.
 *
 * @param format (const char*, Required) — printf-style format string.
 * @param ...    (varargs, Required)    — format arguments.
 */
void print_error(const char* format, ...)
{
#ifdef _WIN32
	HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(console_handle, RED_COLOR);
#endif
	va_list args;

#ifndef _WIN32
	fprintf(stderr, RED_COLOR "[ERROR] ");
#else
	printf("[ERROR] ");
#endif

	va_start(args, format);
	vprintf(format, args);
	va_end(args);

#ifdef _WIN32
	fprintf(stderr, "\nCode : %d)\n", GetLastError());
#else
	fprintf(stderr, "\nCode: %d\t%s\n", errno, strerror(errno));
#endif

#ifndef _WIN32
	fprintf(stderr, RESET_COLOR);
#else
	SetConsoleTextAttribute(console_handle, RESET_COLOR);
#endif
}




/**
 * Prints a success message to stdout in green.
 *
 * @param format (const char*, Required) — printf-style format string.
 * @param ...    (varargs, Required)    — format arguments.
 */
void print_success(const char* format, ...)
{
#ifdef _WIN32
	HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(console_handle, GREEN_COLOR);
#endif
	va_list args;

#ifndef _WIN32
	printf(GREEN_COLOR "[SUCCESS] ");
#else
	printf("[SUCCESS] ");
#endif
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

#ifndef _WIN32
	printf(RESET_COLOR);
#else
	SetConsoleTextAttribute(console_handle, RESET_COLOR);
#endif
}




/**
 * Prints an informational message to stdout in yellow.
 *
 * @param format (const char*, Required) — printf-style format string.
 * @param ...    (varargs, Required)    — format arguments.
 */
void print_info(const char* format, ...)
{
#ifdef _WIN32
	HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(console_handle, YELLOW_COLOR);
#endif
	va_list args;

#ifndef _WIN32
	printf(YELLOW_COLOR "[INFO] ");
#else
	printf("[INFO] ");
#endif
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

#ifndef _WIN32
	printf(RESET_COLOR);
#else
	SetConsoleTextAttribute(console_handle, RESET_COLOR);
#endif
}

