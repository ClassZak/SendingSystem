#include "PrintProcedures/PrintProcedures.h"
#include "FileService/FileService.h"
#include "Base64/Base64.h"

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
#include <sys/select.h>
#include <fcntl.h>
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cJSON.h>


// ---------- Constants and macros ----------
#define BUFFER_SIZE 4096
#define TIMEOUT_SEC 50
#define MAX_MESSAGE_SIZE 0x6400000
#define IP_SOLVER_SERVER_DOMAIN "icanhazip.com"
#define HTTP_REQUEST "GET / HTTP/1.1\r\nHost: icanhazip.com\r\nConnection: close\r\n\r\n"
const unsigned short CONNECTION_PORT = 5000;
const char* CONNECTION_IP = "0.0.0.0";
const int PORT_RECONNECTION_SLEEP_TIME = 5;

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
int receive_message_length(SOCKET client_socket, SOCKET* server_socket, uint32_t* length);


// ---------- Main ----------
int main(int argc, char** argv)
{

#ifdef _WIN32
	setlocale(LC_ALL, "Russian");
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

	while (1)
	{
		// Find an available port starting from connectionPort
		SOCKET* serverSocket = NULL;
		struct sockaddr_in serverAddr;
		do
		{
			serverSocket = setup_socket(AF_INET, SOCK_STREAM, 0,
										&serverAddr, CONNECTION_IP, CONNECTION_PORT);
			if (serverSocket == NULL) {
				print_error("Failed to connect on port %d\nRestart in%d\n", CONNECTION_PORT, PORT_RECONNECTION_SLEEP_TIME);
				sleep(PORT_RECONNECTION_SLEEP_TIME);
			} else {
				break;
			}
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
					  CONNECTION_PORT, ipAddressStr, CONNECTION_PORT);

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
			continue;
		}

		// Print client IP
		char clientIP[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
		print_info("Client connected: %s:%d\n", clientIP, ntohs(clientAddr.sin_port));

		// Set receive timeout (to avoid hanging forever if client misbehaves)
		if (!set_socket_timeout(clientSocket, TIMEOUT_SEC))
		{
			print_error("Failed to set socket timeout\n");
		}

		uint32_t message_length = 0;
		if (receive_message_length(clientSocket, serverSocket, &message_length))
		{
			closesocket(clientSocket);
			closesocket(*serverSocket);
			print_error("Failed to receive message length\n");
			continue;
		}
	
		if (message_length == 0 || message_length > MAX_MESSAGE_SIZE)
		{
			print_error("Invalid message length: %u\n", message_length);
			closesocket(clientSocket);
			closesocket(*serverSocket);
			free(serverSocket);
			continue;
		}


		char* json_buffer = (char*)malloc(message_length+1);
		if (!json_buffer)
		{
			print_error("Memory allocation error\n");
			closesocket(clientSocket);
			closesocket(*serverSocket);
			free(serverSocket);
			continue;

		}
		json_buffer[message_length] = '\0';


		int total_received = 0;
		while (total_received < message_length)
		{
			int n = recv(clientSocket, json_buffer + total_received, message_length - total_received, 0);
			if (n <= 0)
				break;
			total_received += n;
		}
		if (total_received != message_length)
		{
			print_error("Failed to receive complete message\n");
			free(json_buffer);
			closesocket(clientSocket);
			closesocket(*serverSocket);
			free(serverSocket);
			continue;
		}
	
		cJSON* root = cJSON_Parse(json_buffer);
		/* Process the complete received data (should be one JSON object).
		 * If "saveToPath" field is empty or does not exists print the client message.
		 * If "type" is equals "RAW_DATA" save the raw data and decode from base64 instead.
		 * The "type" field is used to describe which kind of data client sended to
		 * but the same time "saveToPath" is used to calculate is need to save data in 
		 * the file or not.
		 * */
		if (root)
		{
			cJSON* typeItem		= cJSON_GetObjectItem(root, "type");
			cJSON* dataItem		= cJSON_GetObjectItem(root, "data");
			cJSON* saveToPath	= cJSON_GetObjectItem(root, "saveToPath");
			if (cJSON_IsString(typeItem) && cJSON_IsString(dataItem) && (!cJSON_IsString(saveToPath) || saveToPath->valuestring[0]=='\0'))
			{
				printf("[%s]: %s\n", typeItem->valuestring, dataItem->valuestring);
				send_json(clientSocket, "message", "OK");
			}
			else if (cJSON_IsString(typeItem) && cJSON_IsString(dataItem) && cJSON_IsString(saveToPath))
			{
				const char *type = typeItem->valuestring;
				const char *data = dataItem->valuestring;
				if (strcmp(type, "RAW_DATA") == 0)
					printf("[%s]: %s\n", typeItem->valuestring, dataItem->valuestring);



				char safe_name[MAX_FILENAME + 1];
				sanitize_filename(safe_name, sizeof(safe_name), saveToPath->valuestring);
#ifdef _WIN32
				_mkdir(SAVE_DIR);
#else
				mkdir(SAVE_DIR, 0755);
#endif
				int max_length = MAX_FILENAME + SAVE_DIR_LENGTH + 1; // <save_directory>/<file_name>
				char full_path[max_length];
				snprintf(full_path, sizeof(full_path), "%s/%s", SAVE_DIR, safe_name);

				FILE* file = fopen(full_path, "wb");
				if (!file)
				{
					print_error("Failed to open %s\n", full_path);
					send_json(clientSocket, "error", "Cannot save file");
				}
				else
				{
					if (strcmp(type, "RAW_DATA") == 0)
					{
						fputs(data, file);
						fclose(file);
						printf("[%s] saved to %s\n", type, full_path);
						send_json(clientSocket, "message", "File saved");
					}
					else if (strcmp(type, "FILE") == 0)
					{
						// Decode Base64 binary data
						size_t b64_len = strlen(data);
						unsigned char *binary = (unsigned char *)malloc(b64_len);
						if (binary)
						{
							int decoded_len = base64_decode(data, binary, b64_len);
							if (decoded_len > 0)
							{
								fwrite(binary, 1, decoded_len, file);
								fclose(file);
								printf("[%s] saved binary file, %d bytes\n", type, decoded_len);
								send_json(clientSocket, "message", "File saved");
							}
							else
							{
								fclose(file);
								print_error("Base64 decode failed\n");
								send_json(clientSocket, "error", "Base64 decode failed");
							}
							free(binary);
						}
						else
						{
							fclose(file);
							print_error("Memory allocation error\n");
							send_json(clientSocket, "error", "Memory error");
						}
					}
					else
					{
						// Unknown type. Save the file
						fputs(data, file);
						fclose(file);
						printf("[%s] saved to %s\n", type, full_path);
						send_json(clientSocket, "message", "File saved");
					}
				}
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
		// Cleanup
		free(json_buffer);
		closesocket(clientSocket);
		closesocket(*serverSocket);
		free(serverSocket);
	}
#ifdef _WIN32
	WSACleanup();
#endif
	free(ipAddressStr);

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
char* get_global_ip(void)
{
	SOCKET sock = INVALID_SOCKET;
	struct addrinfo hints, *result = NULL, *ptr = NULL;
	char *ip = NULL;
	char recvbuf[BUFFER_SIZE];
	int iResult;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if ((iResult = getaddrinfo(IP_SOLVER_SERVER_DOMAIN, "80", &hints, &result)) != 0)
	{
		print_error("getaddrinfo failed: %d\n", iResult);
		return NULL;
	}

	// ---------- 1. Try addresses until connected with timeout ----------
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (sock == INVALID_SOCKET)
			continue;

		// Set socket to non-blocking mode
#ifdef _WIN32
		u_long mode = 1;
		if (ioctlsocket(sock, FIONBIO, &mode) != 0)
		{
			closesocket(sock);
			sock = INVALID_SOCKET;
			continue;
		}
#else
		int flags = fcntl(sock, F_GETFL, 0);
		if (flags == -1 || fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1)
		{
			closesocket(sock);
			sock = INVALID_SOCKET;
			continue;
		}
#endif

		// Non-blocking connect
		if (connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR)
		{
#ifdef _WIN32
			if (WSAGetLastError() != WSAEWOULDBLOCK)
#else
			if (errno != EINPROGRESS)
#endif
			{
				closesocket(sock);
				sock = INVALID_SOCKET;
				continue;
			}
		}

		// Wait with timeout until socket becomes writable
		fd_set write_fds;
		FD_ZERO(&write_fds);
		FD_SET(sock, &write_fds);
		struct timeval tv;
		tv.tv_sec = TIMEOUT_SEC;
		tv.tv_usec = 0;

		int select_ret = select((int)sock + 1, NULL, &write_fds, NULL, &tv);
		if (select_ret <= 0)
		{
			closesocket(sock);
			sock = INVALID_SOCKET;
			continue;
		}

		// Check if socket error occurred
		int so_error = 0;
		socklen_t len = sizeof(so_error);
		if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&so_error, &len) == SOCKET_ERROR ||
			so_error != 0)
		{
			closesocket(sock);
			sock = INVALID_SOCKET;
			continue;
		}

		// Restore blocking mode
#ifdef _WIN32
		mode = 0;
		if (ioctlsocket(sock, FIONBIO, &mode) != 0)
		{
			closesocket(sock);
			sock = INVALID_SOCKET;
			continue;
		}
#else
		if (fcntl(sock, F_SETFL, flags & ~O_NONBLOCK) == -1)
		{
			closesocket(sock);
			sock = INVALID_SOCKET;
			continue;
		}
#endif

		// Successfully connected — exit loop
		break;
	}

	freeaddrinfo(result);

	if (sock == INVALID_SOCKET)
	{
		print_error("Failed to connect to IP resolver\n");
		return NULL;
	}

	// Set receive timeout
	if (!set_socket_timeout(sock, TIMEOUT_SEC))
	{
		print_error("Failed to set receive timeout\n");
		closesocket(sock);
		return NULL;
	}

	// ---------- 2. Send HTTP request with guaranteed full send ----------
	const char *request = HTTP_REQUEST;
	int request_len = (int)strlen(request);
	int sent_total = 0;

	while (sent_total < request_len)
	{
		iResult = send(sock, request + sent_total, request_len - sent_total, 0);
		if (iResult == SOCKET_ERROR)
		{
#ifdef _WIN32
			int err = WSAGetLastError();
#else
			int err = errno;
#endif
			if (err == EINTR)
				continue;
			print_error("Send failed\n");
			closesocket(sock);
			return NULL;
		}
		sent_total += iResult;
	}

	// ---------- 3. Read response respecting Content-Length ----------
	bool headers_parsed = false;
	int content_length = -1;
	char *body_start = NULL;
	int total_received = 0;
	char *response = NULL;

	while (1)
	{
		iResult = recv(sock, recvbuf, sizeof(recvbuf) - 1, 0);
		if (iResult > 0)
		{
			recvbuf[iResult] = '\0';
			char *temp = realloc(response, total_received + iResult + 1);
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

			// Look for the end of headers (empty line)
			if (!headers_parsed)
			{
				body_start = strstr(response, "\r\n\r\n");
				if (body_start)
				{
					headers_parsed = true;
					// Try to find Content-Length
					char *cl = strstr(response, "Content-Length:");
					if (cl)
					{
						cl += strlen("Content-Length:");
						while (*cl == ' ') cl++;
						content_length = (int)strtol(cl, NULL, 10);
					}
					body_start += 4;   // start of body
				}
			}

			// If length is known and the full body is received — exit
			if (headers_parsed && content_length >= 0)
			{
				int body_received = total_received - (int)(body_start - response);
				if (body_received >= content_length)
					break;
			}
		}
		else if (iResult == 0)
		{
			// Server closed connection
			break;
		}
		else
		{
			// Error
#ifdef _WIN32
			int err = WSAGetLastError();
#else
			int err = errno;
#endif
			if (err == EINTR)
				continue;
			// If timeout or other error — stop reading
			break;
		}
	}

	closesocket(sock);
	sock = INVALID_SOCKET;

	// ---------- 4. Extract IP from response body ----------
	if (response)
	{
		char *body = body_start;
		if (!body)  // fallback: if headers weren't found, search manually
		{
			body = strstr(response, "\r\n\r\n");
			if (body) body += 4;
		}

		if (body)
		{
			// Remove trailing spaces and newlines
			char *end = body + strcspn(body, "\r\n\t ");
			size_t len = end - body;
			if (len > 0)
			{
				ip = malloc(len + 1);
				if (ip)
				{
					memcpy(ip, body, len);
					ip[len] = '\0';
				}
			}
		}
		free(response);
	}

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

// ---------- Send JSON response ----------
void send_json(SOCKET sock, const char* type, const char* data)
{
	cJSON* root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "type", type);
	cJSON_AddStringToObject(root, "data", data);
	char* jsonStr = cJSON_PrintUnformatted(root);
	if (jsonStr)
	{
		uint32_t message_length = (uint32_t)strlen(jsonStr);
		uint32_t message_length_net = htonl(message_length);
		send(sock, (char*)&message_length_net, sizeof(message_length_net),0);
		send(sock, jsonStr, message_length, 0);
		free(jsonStr);
	}
	cJSON_Delete(root);
}

// Receive message length
int receive_message_length(SOCKET client_socket, SOCKET* server_socket, uint32_t* length)
{
	int received = 0, total_received = 0;
	while (total_received != sizeof(uint32_t))
	{
		received = recv(client_socket, ((uint8_t*)length) + total_received, sizeof(uint32_t) - total_received, 0);
		if (received <= 0)
		{
			if (received == 0)
				print_error("Client disconnected before sending length\n");
			else
				print_error("recv error while reading length\n");

			return EXIT_FAILURE;
		}
		total_received += received;
	}

	*length = ntohl(*length);

	return EXIT_SUCCESS;
}

