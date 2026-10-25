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
 * Small inline helpers shared by the repository modules.
 */
#pragma once

#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>




/**
 * Returns a heap copy of the text in column @p col.
 *
 * @param s   (sqlite3_stmt*, Required) — statement in SQLITE_ROW state.
 * @param col (int, Required) — 0-based column index.
 * @return (char*) — heap string, or NULL if the column is SQL NULL or
 *         allocation failed. Caller frees.
 */
static inline char* db_dup_col(sqlite3_stmt* s, int col) {
	const unsigned char* t = sqlite3_column_text(s, col);

	if (!t) {
		return NULL;
	}

	size_t len = (size_t)sqlite3_column_bytes(s, col);
	char* out = malloc(len + 1);
	if (out) {
		memcpy(out, t, len);
		out[len] = '\0';
	}

	return out;
}




/**
 * Binds a text parameter, or SQL NULL when @p v is NULL.
 *
 * @param s   (sqlite3_stmt*, Required) — statement.
 * @param idx (int, Required) — 1-based parameter index.
 * @param v   (const char*, Optional) — text value; NULL binds SQL NULL.
 */
static inline void db_bind_text_or_null(sqlite3_stmt* s, int idx, const char* v) {
	if (v) {
		sqlite3_bind_text(s, idx, v, -1, SQLITE_TRANSIENT);
	} else {
		sqlite3_bind_null(s, idx);
	}
}
