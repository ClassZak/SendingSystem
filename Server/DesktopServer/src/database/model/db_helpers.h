#pragma once
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>

/* Returns a heap copy of column @p col, or NULL. */
static inline char* db_dup_col(sqlite3_stmt* s, int col) {
	const unsigned char* t = sqlite3_column_text(s, col);

	if (!t)
		return NULL;

	size_t len = (size_t)sqlite3_column_bytes(s, col);
	char* out = malloc(len + 1);
	if (out) {
		memcpy(out, t, len);
		out[len] = '\0';
	}

	return out;
}

/* Binds text or NULL. */
static inline void db_bind_text_or_null(sqlite3_stmt* s, int idx, const char* v) {
	if (v)
		sqlite3_bind_text(s, idx, v, -1, SQLITE_TRANSIENT);
	else
		sqlite3_bind_null(s, idx);
}
