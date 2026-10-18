#include "set_socket_timeout.h"

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <Windows.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/select.h>
#include <fcntl.h>
#include <pthread.h>
#endif


typedef int SOCKET;




bool set_socket_timeout(SOCKET sock, int timeout_sec)
{
	struct timeval tv;
	tv.tv_sec = timeout_sec;
	tv.tv_usec = 0;

	return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
					  (const char*)&tv, sizeof(tv)) == 0;
}

