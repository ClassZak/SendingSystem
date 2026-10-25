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
 * C structs mirroring the Android Room entities.
 *
 * Field names are snake_case in C; SQL columns stay camelCase to match
 * Room. Every heap-owned string must be released with the matching
 * *_free / *_free_all function.
 */
#pragma once

#include "../database_config.h"




typedef struct {
	DB_ID id;
	char* name;   /**< owned; may be NULL */
} MessageType;




typedef struct {
	DB_ID id;
	char* name;   /**< owned; may be NULL */
} MessageStatus;




typedef struct {
	DB_ID id;
	char* ip;                /**< owned; may be NULL */
	int   port;
	int   is_ssl_encrypted;  /**< 0 or 1 */
} Connection;




typedef struct {
	DB_ID id;
	DB_ID connection_id;
	DB_ID message_status_id;
	DB_ID message_type_id;
	char* response;      /**< owned; may be NULL */
	char* sending_time;  /**< owned; may be NULL */
	char* request;       /**< owned; may be NULL */
} Message;




typedef struct {
	DB_ID id;
	char* name;          /**< owned; may be NULL */
	DB_ID connection_id;
} NamedConnection;




/**
 * Releases the heap strings inside a MessageType.
 *
 * @param t (MessageType*, Optional) — NULL is safe.
 */
void message_type_free(MessageType* t);




/**
 * Frees an array returned by message_type_get_all.
 *
 * @param arr (MessageType*, Optional) — NULL is safe.
 * @param n   (int, Required) — number of elements.
 */
void message_type_free_all(MessageType* arr, int n);




/**
 * Releases the heap strings inside a MessageStatus.
 *
 * @param s (MessageStatus*, Optional) — NULL is safe.
 */
void message_status_free(MessageStatus* s);




/**
 * Frees an array returned by message_status_get_all.
 *
 * @param arr (MessageStatus*, Optional) — NULL is safe.
 * @param n   (int, Required) — number of elements.
 */
void message_status_free_all(MessageStatus* arr, int n);




/**
 * Releases the heap strings inside a Connection.
 *
 * @param c (Connection*, Optional) — NULL is safe.
 */
void connection_free(Connection* c);




/**
 * Frees an array returned by connection_get_all.
 *
 * @param arr (Connection*, Optional) — NULL is safe.
 * @param n   (int, Required) — number of elements.
 */
void connection_free_all(Connection* arr, int n);




/**
 * Releases the heap strings inside a Message.
 *
 * @param m (Message*, Optional) — NULL is safe.
 */
void message_free(Message* m);




/**
 * Frees an array returned by message_get_all.
 *
 * @param arr (Message*, Optional) — NULL is safe.
 * @param n   (int, Required) — number of elements.
 */
void message_free_all(Message* arr, int n);




/**
 * Releases the heap strings inside a NamedConnection.
 *
 * @param nc (NamedConnection*, Optional) — NULL is safe.
 */
void named_connection_free(NamedConnection* nc);




/**
 * Frees an array returned by named_connection_get_all.
 *
 * @param arr (NamedConnection*, Optional) — NULL is safe.
 * @param n   (int, Required) — number of elements.
 */
void named_connection_free_all(NamedConnection* arr, int n);
