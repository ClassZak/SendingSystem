#include "setup_socket.h"
#include "../PrintProcedures/PrintProcedures.h"


SOCKET* setup_socket(int af, int type, int protocol,
								   struct sockaddr_in* server_addr,
								   const char* ip, unsigned short port)
{
	SOCKET* server_sock = malloc(sizeof(SOCKET));
	if (!server_sock)
	{
		print_error("Memory allocation error for socket\n");
		return NULL;
	}
	*server_sock = socket(af, type, protocol);
	if (*server_sock == INVALID_SOCKET)
	{
		print_error("Socket creation error\n");
		free(server_sock);
		return NULL;
	}

	memset(server_addr, 0, sizeof(struct sockaddr_in));
	server_addr->sin_family = af;
	inet_pton(af, ip, &server_addr->sin_addr);
	server_addr->sin_port = htons(port);

	int optval = 1;
	setsockopt(*server_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));
	if (bind(*server_sock, (struct sockaddr*)server_addr,
			 sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		print_error("Bind failed on port %d\n", port);
		closesocket(*server_sock);
		free(server_sock);
		return NULL;
	}

	return server_sock;
}

