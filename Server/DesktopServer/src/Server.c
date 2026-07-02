#include "PrintProcedures/PrintProcedures.h"

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <Windows.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#endif

#include <stdarg.h>
#include <locale.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "cJSON.h"


// ---------- Constants and macros ----------
#define BUFFER_SIZE 4096
#define TIMEOUT_SEC 50
#define HTTP_REQUEST "GET / HTTP/1.1\r\nHost: icanhazip.com\r\nConnection: close\r\n\r\n"

#ifndef _WIN32
typedef int SOCKET;
#define closesocket(s) close(s)
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR   (-1)
#endif

// ---------- Forward declarations ----------
bool set_socket_timeout(SOCKET sock, int timeout_sec);
char* get_global_ip(void);
static inline SOCKET* setup_socket
(
	int af, int type, int protocol,
	struct sockaddr_in* server_addr,
	const char* ip, unsigned short port
);
void send_json(SOCKET sock, const char* type, const char* data);

// ---------- Global variables ----------
unsigned short connectionPort = 5000;
const char* connectionIp = "0.0.0.0";

// ---------- Main ----------
int main(int argc, char** argv)
{
	setlocale(LC_ALL, "Russian");

#ifdef _WIN32
	WSADATA wsaData;
	int startupResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (startupResult)
	{
		print_error("WinSock initialization failed\n");
		exit(startupResult);
	}
	print_info("Server initialization\n");
#endif

	// Determine external IP (informational only)
	char* ipAddressStr = get_global_ip();
	if (!ipAddressStr)
	{
		print_error("Failed to obtain external IP\n");
		return 1;
	}
	print_info("Server external IP: %s\n", ipAddressStr);

	// Find an available port starting from connectionPort
	SOCKET* serverSocket = NULL;
	struct sockaddr_in serverAddr;
	do
	{
		serverSocket = setup_socket(AF_INET, SOCK_STREAM, 0,
									&serverAddr, connectionIp, connectionPort);
		if (serverSocket != NULL)
		{
			break;
		}
		++connectionPort;
	}
	while (true);

	if (listen(*serverSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		print_error("Listen failed\n");
		closesocket(*serverSocket);
		free(serverSocket);
#ifdef _WIN32
		WSACleanup();
#endif
		exit(EXIT_FAILURE);
	}

	print_success("Server started and waiting for connections on port %d (%s:%d)\n",
				  connectionPort, ipAddressStr, connectionPort);
	free(ipAddressStr);

	// Accept one connection
	SOCKET clientSocket;
	struct sockaddr_in clientAddr;
	socklen_t clientAddrSize = sizeof(clientAddr);
	if ((clientSocket = accept(*serverSocket, (struct sockaddr*)&clientAddr,
							   &clientAddrSize)) == INVALID_SOCKET)
	{
		print_error("Accept failed\n");
		closesocket(*serverSocket);
		free(serverSocket);
#ifdef _WIN32
		WSACleanup();
#endif
		exit(EXIT_FAILURE);
	}

	// Print client IP
	char clientIP[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
	printf("Client connected: %s:%d\n", clientIP, ntohs(clientAddr.sin_port));

	// Set receive timeout (to avoid hanging forever if client misbehaves)
	if (!set_socket_timeout(clientSocket, TIMEOUT_SEC))
	{
		print_error("Failed to set socket timeout\n");
	}

	// Data processing loop – read until client closes write side
	size_t bigBufSize = 0;
	char* bigBuffer = NULL;

	while (1)
	{
		char chunk[BUFFER_SIZE];
		int bytesReceived = recv(clientSocket, chunk, BUFFER_SIZE - 1, 0);

		if (bytesReceived > 0)
		{
			// Append to dynamic buffer
			char* temp = realloc(bigBuffer, bigBufSize + bytesReceived + 1);
			if (!temp)
			{
				print_error("Memory allocation error\n");
				free(bigBuffer);
				bigBuffer = NULL;
				bigBufSize = 0;
				break;
			}
			bigBuffer = temp;
			memcpy(bigBuffer + bigBufSize, chunk, bytesReceived);
			bigBufSize += bytesReceived;
			bigBuffer[bigBufSize] = '\0';
		}
		else if (bytesReceived == 0)
		{
			print_info("Client disconnected\n");
			break;
		}
		else
		{
#ifdef _WIN32
			if (WSAGetLastError() == WSAETIMEDOUT)
			{
				print_error("Connection timed out\n");
			}
			else
			{
				print_error("Receive error\n");
			}
#else
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				print_error("Connection timed out\n");
			}
			else
			{
				print_error("Receive error\n");
			}
#endif
			break;
		}
	}

	// Process the complete received data (should be one JSON object)
	if (bigBuffer && bigBufSize > 0)
	{
		cJSON* root = cJSON_Parse(bigBuffer);
		if (root)
		{
			cJSON* typeItem = cJSON_GetObjectItem(root, "type");
			cJSON* dataItem = cJSON_GetObjectItem(root, "data");
			if (cJSON_IsString(typeItem) && cJSON_IsString(dataItem))
			{
				printf("[%s]: %s\n", typeItem->valuestring, dataItem->valuestring);
				send_json(clientSocket, "message", "OK");
			}
			else
			{
				print_error("JSON missing 'type' or 'data' field\n");
				send_json(clientSocket, "error", "Invalid format");
			}
			cJSON_Delete(root);
		}
		else
		{
			print_error("Failed to parse JSON\n");
			send_json(clientSocket, "error", "JSON parse error");
		}
	}
	else
	{
		print_error("No data received\n");
	}

	// Cleanup
	free(bigBuffer);
	closesocket(clientSocket);
	closesocket(*serverSocket);
	free(serverSocket);
#ifdef _WIN32
	WSACleanup();
#endif

	return EXIT_SUCCESS;
}

// ---------- Set socket timeout ----------
bool set_socket_timeout(SOCKET sock, int timeout_sec)
{
	struct timeval tv;
	tv.tv_sec = timeout_sec;
	tv.tv_usec = 0;

	return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
					  (const char*)&tv, sizeof(tv)) == 0;
}

// ---------- Obtain global IP via icanhazip.com ----------
char* get_global_ip()
{
	SOCKET sock = INVALID_SOCKET;
	struct addrinfo hints, *result = NULL, *ptr = NULL;
	char* ip = NULL;
	char recvbuf[BUFFER_SIZE];
	int iResult;

#ifdef _WIN32
	ZeroMemory(&hints, sizeof(hints));
#else
	memset(&hints, 0, sizeof(hints));
#endif
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if ((iResult = getaddrinfo("icanhazip.com", "80", &hints, &result)) != 0)
	{
		fprintf(stderr, "getaddrinfo failed: %d\n", iResult);
		return NULL;
	}

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (sock == INVALID_SOCKET)
		{
#ifdef _WIN32
			fprintf(stderr, "socket failed: %d\n", WSAGetLastError());
#else
			print_error("socket failed: %d\t%s\n", errno, strerror(errno));
#endif
			continue;
		}
		if (connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR)
		{
			closesocket(sock);
			sock = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (sock == INVALID_SOCKET)
	{
		fprintf(stderr, "Failed to connect to receive the IP\n");
		return NULL;
	}

	if ((iResult = send(sock, HTTP_REQUEST, (int)strlen(HTTP_REQUEST), 0)) == SOCKET_ERROR)
	{
#ifdef _WIN32
		fprintf(stderr, "Send failed: %d\n", WSAGetLastError());
#else
		print_error("Sending failed: %d\t%s\n", errno, strerror(errno));
#endif
		closesocket(sock);
		return NULL;
	}

	// Receive response
	int total_received = 0;
	char* response = NULL;
	do
	{
		iResult = recv(sock, recvbuf, BUFFER_SIZE - 1, 0);
		if (iResult > 0)
		{
			recvbuf[iResult] = '\0';
			char* temp = realloc(response, total_received + iResult + 1);
			if (!temp)
			{
				free(response);
				closesocket(sock);
				return NULL;
			}
			response = temp;
			memcpy(response + total_received, recvbuf, iResult);
			total_received += iResult;
			response[total_received] = '\0';
		}
		else if (iResult == 0)
		{
			break;
		}
		else
		{
#ifdef _WIN32
			fprintf(stderr, "recv failed: %d\n", WSAGetLastError());
#else
			print_error("Receiving failed: %d\t%s\n", errno, strerror(errno));
#endif
			free(response);
			closesocket(sock);
			return NULL;
		}
	}
	while (iResult > 0);

	if (response)
	{
		char* body = strstr(response, "\r\n\r\n");
		if (body)
		{
			body += 4;
			char* end = body + strcspn(body, "\r\n\t ");
			size_t len = end - body;
			ip = malloc(len + 1);
			if (ip)
			{
				strncpy(ip, body, len);
				ip[len] = '\0';
			}
		}
		free(response);
	}

	closesocket(sock);

	return ip;
}

// ---------- Socket creation and binding ----------
static inline SOCKET* setup_socket(int af, int type, int protocol,
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

// ---------- Send JSON response ----------
void send_json(SOCKET sock, const char* type, const char* data)
{
	cJSON* root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "type", type);
	cJSON_AddStringToObject(root, "data", data);
	char* jsonStr = cJSON_PrintUnformatted(root);
	if (jsonStr)
	{
		send(sock, jsonStr, (int)strlen(jsonStr), 0);
		free(jsonStr);
	}
	cJSON_Delete(root);
}

