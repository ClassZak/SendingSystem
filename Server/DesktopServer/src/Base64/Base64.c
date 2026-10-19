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
#include "Base64.h"



#include <stdint.h>
#include <string.h>

static int b64_value(char c)
{
	if (c >= 'A' && c <= 'Z')
		return c - 'A';
	if (c >= 'a' && c <= 'z')
		return c - 'a' + 26;
	if (c >= '0' && c <= '9')
		return c - '0' + 52;
	if (c == '+')
		return 62;
	if (c == '/')
		return 63;
	return -1; // illegal
}

int base64_decode(const char *in, unsigned char *out, int out_len)
{
	int out_pos = 0;
	int buf[4];
	int buf_pos = 0;
	int padding = 0;

	for (const char *p = in; *p; p++)
	{
		if (*p == '=')
		{
			// '=' allowed at end
			if (padding > 1) return -1; // more than 2 '='
			padding++;
			buf[buf_pos++] = 0; // filler
		}
		else
		{
			int val = b64_value(*p);
			if (val < 0)
				return -1;
			if (padding > 0)
				return -1;
			buf[buf_pos++] = val;
		}

		if (buf_pos == 4)
		{
			if (out_pos + (3 - padding) > out_len)
				return -1; // buffer overflow
			out[out_pos++] = (unsigned char)((buf[0] << 2) | (buf[1] >> 4));
			if (padding < 2)
				out[out_pos++] = (unsigned char)((buf[1] << 4) | (buf[2] >> 2));
			if (padding < 1)
				out[out_pos++] = (unsigned char)((buf[2] << 6) | buf[3]);
			buf_pos = 0;
			padding = 0;
		}
	}

	if (buf_pos != 0)
		return -1;

	return out_pos;
}
