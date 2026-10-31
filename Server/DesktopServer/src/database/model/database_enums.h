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
 * Numeric ids for the reference tables. Values must match the seed
 * rows in CreateDatabase.sql.
 *
 * Use these instead of bare integers when calling repository
 * functions — a typo like 3 vs 4 becomes a compile error instead of
 * a row pointing at the wrong role.
 */
#pragma once




/**
 * Values of MessageType.id.
 */
typedef enum {
	MESSAGE_TYPE_RAW_DATA = 1,
	MESSAGE_TYPE_FILE     = 2,
} MessageTypeId;




/**
 * Values of MessageStatus.id.
 */
typedef enum {
	MESSAGE_STATUS_SUCCESS = 1,
	MESSAGE_STATUS_FAILED  = 2,
} MessageStatusId;




/**
 * Values of MessageDirection.id.
 */
typedef enum {
	MESSAGE_DIRECTION_OUTBOUND = 1,
	MESSAGE_DIRECTION_INBOUND  = 2,
} MessageDirectionId;




/**
 * Values of ConnectionRole.id.
 *
 * Note: this is the *id enum*, not the row struct. The row struct is
 * `ConnectionRole` in database_models.h — a different symbol.
 */
typedef enum {
	CONNECTION_ROLE_SERVER = 1,
	CONNECTION_ROLE_CLIENT = 2,
	CONNECTION_ROLE_MASTER = 3,
	CONNECTION_ROLE_SLAVE  = 4,
} ConnectionRoleId;
