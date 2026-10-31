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
 * Database path and row-id type.
 */
#pragma once




/**
 * Row identifier. SQLite rowid is 64-bit. On LP64 (Linux, macOS) long
 * matches; on Windows 64-bit long is 32-bit — switch to int64_t
 * (<stdint.h>) if you build the server there.
 */
typedef long DB_ID;




/** Path to the SQLite database file. */
static const char* DATABASE_FILENAME = "SendingSystem.db";
/** Path to the SQL schema script applied on startup. */
static const char* CREATE_DATABASE_FILE = "CreateDatabase.sql";
