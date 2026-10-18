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

