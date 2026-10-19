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
#include "receive_message_length.h"
#include <stdlib.h>
#include "../PrintProcedures/PrintProcedures.h"
#include "config.h"


int receive_message_length(connection_t* connection, uint32_t* length, pthread_mutex_t* mtx_print)
{
	if (length == NULL || !connection || !connection->ctx || !connection->read)
		return EXIT_FAILURE;

	int received = 0, total_received = 0;
	while (total_received != sizeof(uint32_t))
	{
		received = connection->read(connection->ctx,((uint8_t*)length + total_received), sizeof(uint32_t) - total_received);
		if (received <= 0)
		{
			pthread_mutex_lock(mtx_print);
			if (received == 0)
				print_error("Client disconnected before sending length\n");
			else
				print_error("recv error while reading length\n");
			pthread_mutex_unlock(mtx_print);

			return EXIT_FAILURE;
		}
		total_received += received;
	}

	*length = ntohl(*length);

	return EXIT_SUCCESS;
}

