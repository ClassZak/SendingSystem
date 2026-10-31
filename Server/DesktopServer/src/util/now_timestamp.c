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

#include <time.h>

/**
 * Writes the current local time into the buffer as
 * "yyyy-MM-dd HH:mm:ss" — the same format the Android client stores
 * in Message.sendingTime, so rows on both sides sort identically.
 *
 * @param buffer          (char*, Required) — output buffer.
 * @param buffer_capacity (size_t, Required) — capacity; must fit 19
 *                        characters plus NUL.
 */
void now_timestamp(char* buffer, size_t buffer_capacity) {
	time_t now = time(NULL);
	struct tm local_time;

#ifdef _WIN32
	localtime_s(&local_time, &now);
#else
	localtime_r(&now, &local_time);
#endif

	strftime(buffer, buffer_capacity, "%Y-%m-%d %H:%M:%S", &local_time);
}
