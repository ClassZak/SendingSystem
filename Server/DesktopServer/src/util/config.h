#pragma once
// Definitions
#define BUFFER_SIZE 4096
#define TIMEOUT_SEC 50
#define MAX_MESSAGE_SIZE 0x6400000
#define IP_SOLVER_SERVER_DOMAIN "icanhazip.com"
#define HTTP_REQUEST "GET / HTTP/1.1\r\nHost: icanhazip.com\r\nConnection: close\r\n\r\n"
#define FAILED_SIGNAL_HANDLERS_MESSAGE "Failed to set up signal handlers\n"
#define FAILED_TO_SET_THREAD_ATTRS "Failed to set up thread attributes\n"



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
#include <errno.h>
#include <sys/select.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#endif


#ifndef _WIN32
typedef int SOCKET;
#define closesocket(s) close(s)
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR   (-1)
#endif


#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

