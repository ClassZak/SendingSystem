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
 * Free functions for the row structs.
 *
 * Every struct owns its heap strings; the *_free functions release
 * them and clear the pointers.
 */

#include "database_models.h"


#include <stdlib.h>




/**
 * Frees *p and clears the pointer.
 *
 * @param p (char**, Required) — pointer to the heap string.
 */
static inline void free_str(char** p) {
	if (*p) {
		free(*p);
		*p = NULL;
	}
}




/**
 * Releases the heap strings inside a MessageType.
 *
 * @param t (MessageType*, Optional) — NULL is safe.
 */
void message_type_free(MessageType* t) {
	if (t) {
		free_str(&t->name);
	}
}




/**
 * Frees an array returned by message_type_get_all.
 *
 * @param arr (MessageType*, Optional) — NULL is safe.
 * @param n   (int, Required) — number of elements.
 */
void message_type_free_all(MessageType* arr, int n) {
	if (!arr) {
		return;
	}

	for (int i = 0; i < n; i++) {
		message_type_free(&arr[i]);
	}

	free(arr);
}




/**
 * Releases the heap strings inside a MessageStatus.
 *
 * @param s (MessageStatus*, Optional) — NULL is safe.
 */
void message_status_free(MessageStatus* s) {
	if (s) {
		free_str(&s->name);
	}
}




/**
 * Frees an array returned by message_status_get_all.
 *
 * @param arr (MessageStatus*, Optional) — NULL is safe.
 * @param n   (int, Required) — number of elements.
 */
void message_status_free_all(MessageStatus* arr, int n) {
	if (!arr) {
		return;
	}

	for (int i = 0; i < n; i++) {
		message_status_free(&arr[i]);
	}

	free(arr);
}




/**
 * Releases the heap strings inside a Connection.
 *
 * @param c (Connection*, Optional) — NULL is safe.
 */
void connection_free(Connection* c) {
	if (c) {
		free_str(&c->ip);
	}
}




/**
 * Frees an array returned by connection_get_all.
 *
 * @param arr (Connection*, Optional) — NULL is safe.
 * @param n   (int, Required) — number of elements.
 */
void connection_free_all(Connection* arr, int n) {
	if (!arr) {
		return;
	}

	for (int i = 0; i < n; i++) {
		connection_free(&arr[i]);
	}

	free(arr);
}




/**
 * Releases the heap strings inside a Message.
 *
 * @param m (Message*, Optional) — NULL is safe.
 */
void message_free(Message* m) {
	if (!m) {
		return;
	}

	free_str(&m->response);
	free_str(&m->sending_time);
	free_str(&m->request);
}




/**
 * Frees an array returned by message_get_all.
 *
 * @param arr (Message*, Optional) — NULL is safe.
 * @param n   (int, Required) — number of elements.
 */
void message_free_all(Message* arr, int n) {
	if (!arr) {
		return;
	}

	for (int i = 0; i < n; i++) {
		message_free(&arr[i]);
	}

	free(arr);
}




/**
 * Releases the heap strings inside a NamedConnection.
 *
 * @param nc (NamedConnection*, Optional) — NULL is safe.
 */
void named_connection_free(NamedConnection* nc) {
	if (nc) {
		free_str(&nc->name);
	}
}




/**
 * Frees an array returned by named_connection_get_all.
 *
 * @param arr (NamedConnection*, Optional) — NULL is safe.
 * @param n   (int, Required) — number of elements.
 */
void named_connection_free_all(NamedConnection* arr, int n) {
	if (!arr) {
		return;
	}

	for (int i = 0; i < n; i++) {
		named_connection_free(&arr[i]);
	}

	free(arr);
}

