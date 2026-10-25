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
#include "database_models.h"
#include <stdlib.h>




#define FREE_STR(p) do { free((p)); (p) = NULL; } while (0)




void message_type_free(MessageType* t) {
	if (t)
		FREE_STR(t->name);
}
void message_type_free_all(MessageType* a, int n) {
	if (!a)
		return;

	for (int i = 0; i < n; i++)
		message_type_free(&a[i]);

	free(a);
}




void message_status_free(MessageStatus* s) {
	if (s)
		FREE_STR(s->name);
}
void message_status_free_all(MessageStatus* a, int n) {
	if (!a)
		return;

	for (int i = 0; i < n; i++) message_status_free(&a[i]);

	free(a);
}




void connection_free(Connection* c) {
	if (c)
		FREE_STR(c->ip);
}
void connection_free_all(Connection* a, int n) {
	if (!a)
		return;

	for (int i = 0; i < n; i++)
		connection_free(&a[i]);

	free(a);
}




void message_free(Message* m) {
	if (!m)
		return;

	FREE_STR(m->response);
	FREE_STR(m->sending_time);
	FREE_STR(m->request);
}
void message_free_all(Message* a, int n) {
	if (!a)
		return;

	for (int i = 0; i < n; i++)
		message_free(&a[i]);

	free(a);
}




void named_connection_free(NamedConnection* nc) {
	if (nc)
		FREE_STR(nc->name);
}
void named_connection_free_all(NamedConnection* a, int n) {
	if (!a)
		return;

	for (int i = 0; i < n; i++)
		named_connection_free(&a[i]);

	free(a);
}

