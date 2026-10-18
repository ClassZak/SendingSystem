#pragma once
#include "../Connection/connection.h"
#include <stdint.h>
#include <pthread.h>

int receive_message_length(connection_t* connection, uint32_t* length, pthread_mutex_t* mtx_print);
