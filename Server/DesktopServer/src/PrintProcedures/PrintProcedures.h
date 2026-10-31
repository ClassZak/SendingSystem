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
 * Coloured console output helpers.
 *
 * Only PrintProcedures.c calls fprintf/printf directly; every other
 * module must use these functions. In threaded contexts, hold
 * mtx_print around the call.
 */
#pragma once

/* ANSI colours — Windows needs SetConsoleTextAttribute, POSIX uses
 * ANSI escape sequences. */
#ifdef _WIN32
#include <windows.h>
#endif


#ifdef _WIN32

#define FOREGROUND_DEFAULT	(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define RED_COLOR			FOREGROUND_RED
#define GREEN_COLOR			FOREGROUND_GREEN
#define YELLOW_COLOR		0xE
#define RESET_COLOR			FOREGROUND_DEFAULT

#else

#define RED_COLOR			"\x1b[31m"
#define GREEN_COLOR			"\x1b[32m"
#define YELLOW_COLOR		"\x1b[33m"
#define RESET_COLOR			"\033[0m"

#endif


#include <stdarg.h>
#include <stdio.h>




/**
 * Prints an error message to stderr in red.
 *
 * On POSIX appends `\nCode: <errno>\t<strerror>`; on Windows appends
 * `\nCode : <GetLastError()>`.
 *
 * @param format (const char*, Required) — printf-style format string.
 * @param ...    (varargs, Required)    — format arguments.
 */
void print_error(const char* format, ...);




/**
 * Prints a success message to stdout in green.
 *
 * @param format (const char*, Required) — printf-style format string.
 * @param ...    (varargs, Required)    — format arguments.
 */
void print_success(const char* format, ...);




/**
 * Prints an informational message to stdout in yellow.
 *
 * @param format (const char*, Required) — printf-style format string.
 * @param ...    (varargs, Required)    — format arguments.
 */
void print_info(const char* format, ...);
