#pragma once
#include "config.h"

SOCKET* setup_socket(	int af, int type, int protocol,
						struct sockaddr_in* server_addr,
						const char* ip, unsigned short port);
