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
#pragma once

#include "../database_config.h"




/* Field names are snake_case in C, columns stay camelCase in SQL.
 * Every string is heap-owned: call *_free() / *_free_all(). */
typedef struct {
	DB_ID id;
	char* name;
} MessageType;


typedef struct {
	DB_ID id;
	char* name;
} MessageStatus;


typedef struct {
	DB_ID id;
	char* ip;
	int   port;
	int   is_ssl_encrypted;
} Connection;


typedef struct {
	DB_ID id;
	DB_ID connection_id;
	DB_ID message_status_id;
	DB_ID message_type_id;
	char* response;
	char* sending_time;
	char* request;
} Message;


typedef struct {
	DB_ID id;
	char* name;
	DB_ID connection_id;
} NamedConnection;




void message_type_free(MessageType* t);
void message_type_free_all(MessageType* arr, int n);


void message_status_free(MessageStatus* s);
void message_status_free_all(MessageStatus* arr, int n);


void connection_free(Connection* c);
void connection_free_all(Connection* arr, int n);


void message_free(Message* m);
void message_free_all(Message* arr, int n);


void named_connection_free(NamedConnection* nc);
void named_connection_free_all(NamedConnection* arr, int n);
