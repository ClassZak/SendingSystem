#pragma once
#include <stdbool.h>
// Definitions

/* ---------- Network------- */
#define CONNECTION_PORT  5000
static const char* CONNECTION_IP = "0.0.0.0";

/* ---------- TLS ---------- */
static const bool SSL_ENCRYPT		= true;
static const char* PUBLIC_KEY_FILE	= "server.pem";
static const char* PRIVATE_KEY_FILE	= "server.key";

/* ---------- Limits ------- */
#define MAX_MESSAGE_SIZE  0x6400000   /* 100 MB — body limit /api/send */
#define MAX_UPLOAD_SIZE   0x64000000  /* 1 GB   — file limit		   */
#define REQUEST_TIMEOUT_MS 30000

/* ---------- Utils -------- */
#define FAILED_SIGNAL_HANDLERS_MESSAGE "Failed to set up signal handlers\n"



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

