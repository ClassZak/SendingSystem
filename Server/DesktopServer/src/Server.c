/*
 * Server.c
 * Written by ClassZak
 * */

#include "PrintProcedures/PrintProcedures.h"
#include "FileService/FileService.h"
#include "Base64/Base64.h"
#include "OpenSSL/OpenSSL.h"

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
#include <pthread.h>
#include <signal.h>
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


/* External dependencies */
#include <cJSON.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/crypto.h>




// ---------- Constants and macros ----------
#define BUFFER_SIZE 4096
#define TIMEOUT_SEC 50
#define MAX_MESSAGE_SIZE 0x6400000
#define IP_SOLVER_SERVER_DOMAIN "icanhazip.com"
#define HTTP_REQUEST "GET / HTTP/1.1\r\nHost: icanhazip.com\r\nConnection: close\r\n\r\n"
#define FAILED_SIGNAL_HANDLERS_MESSAGE "Failed to set up signal handlers\n"
#define FAILED_TO_SET_THREAD_ATTRS "Failed to set up thread attributes\n"
const unsigned short CONNECTION_PORT = 5000;
const char* CONNECTION_IP = "0.0.0.0";
const int PORT_RECONNECTION_SLEEP_TIME = 5;
const bool SSL_ENCRYPT = true;
const char* PUBLIC_KEY_FILE = "server.crt";
const char* PRIVATE_KEY_FILE = "server.key";

#ifndef _WIN32
typedef int SOCKET;
#define closesocket(s) close(s)
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR   (-1)
#endif




/* Multithread prepare */
/* Client service thread */
void* handle_client(void* arg);
pthread_mutex_t mtx_print = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx_fprint = PTHREAD_MUTEX_INITIALIZER;
volatile sig_atomic_t keep_running = 1;
#include "Connection/connection.h"
#include "Connection/connection_socket.h"
#include "Connection/connection_SSL.h"



// ---------- Forward declarations ----------
bool set_socket_timeout(SOCKET sock, int timeout_sec);
char* get_global_ip(void);
static inline SOCKET* setup_socket
(
	int af, int type, int protocol,
	struct sockaddr_in* server_addr,
	const char* ip, unsigned short port
);
void send_json(connection_t* connection, const char* type, const char* data);
int receive_message_length(connection_t* connection, uint32_t* length);



/* Signal handling */
int set_up_signals_handling();
void signal_handler(int signal_number);




// ---------- Main ----------
int main(int argc, char** argv)
{
	print_pwd();
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
#else
	/* Assign signal handler to signal */
	if (set_up_signals_handling()) {
		print_error(FAILED_SIGNAL_HANDLERS_MESSAGE);
		return EXIT_FAILURE;
	}
#endif
	/* Set up the SSL */
	SSL_CTX* server_SSL_ctx = NULL;
	if (SSL_ENCRYPT) {
		SSL_library_init();
		OpenSSL_add_all_algorithms();
		SSL_load_error_strings();

		if ((server_SSL_ctx = set_up_server_SSL_certificate(PUBLIC_KEY_FILE, PRIVATE_KEY_FILE)) == NULL) {
			print_all_SSL_errors();
			return EXIT_FAILURE;
		}

		print_success("SSL context was installed correctly\n");
	}

	// Determine external IP (informational only)
	char* ipAddressStr = get_global_ip();
	if (!ipAddressStr)
	{
		print_error("Failed to obtain external IP\n");
		return 1;
	}
	print_info("Server external IP: %s\n", ipAddressStr);


	// Create server socket and try reconnect to port
	SOCKET* serverSocket = NULL;
	struct sockaddr_in serverAddr;
	do
	{
		serverSocket = setup_socket(AF_INET, SOCK_STREAM, 0,
									&serverAddr, CONNECTION_IP, CONNECTION_PORT);
		if (serverSocket == NULL) {
			print_error("Failed to connect on port %d\nRestart in%d\n",
						CONNECTION_PORT, PORT_RECONNECTION_SLEEP_TIME);
			sleep(PORT_RECONNECTION_SLEEP_TIME);
		} else {
			break;
		}
	}
	while (true);
	// Attempt to listen the port
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


	/* Prepare multithread */
	pthread_t pthread;
	pthread_attr_t pthread_attr;
	if (pthread_attr_init(&pthread_attr) != 0) {
		print_error(FAILED_TO_SET_THREAD_ATTRS);
		return EXIT_FAILURE;
	}
	if (pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED)) {
		print_error(FAILED_TO_SET_THREAD_ATTRS);
		return EXIT_FAILURE;
	}

	while (keep_running)
	{
		// Accept one connection
		SOCKET clientSocket;
		struct sockaddr_in clientAddr;
		socklen_t clientAddrSize = sizeof(clientAddr);
		if ((clientSocket = accept(*serverSocket, (struct sockaddr*)&clientAddr,
								   &clientAddrSize)) == INVALID_SOCKET)
		{
			if (errno == EINTR) continue;
			pthread_mutex_lock(&mtx_print);
			print_error("Accept failed\n");
			pthread_mutex_unlock(&mtx_print);
			continue;
		}

		// Print client IP
		char clientIP[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
		pthread_mutex_lock(&mtx_print);
		print_info("Client connected: %s:%d\n", clientIP, ntohs(clientAddr.sin_port));
		pthread_mutex_unlock(&mtx_print);

		// Set receive timeout (to avoid hanging forever if client misbehaves)
		if (!set_socket_timeout(clientSocket, TIMEOUT_SEC))
		{
			pthread_mutex_lock(&mtx_print);
			print_error("Failed to set socket timeout\n");
			pthread_mutex_unlock(&mtx_print);
		}


		// Start client thread
		connection_t* client_connection;
		client_connection = malloc(sizeof(connection_t));
		if (client_connection == NULL) {
			pthread_mutex_lock(&mtx_print);
			print_error("Failed to allocate memory for thread argument\n");
			pthread_mutex_unlock(&mtx_print);
			continue;
		}
		if (!SSL_ENCRYPT) {
			*client_connection = create_connection_socket();
			connection_socket_ctx* ctx;
			ctx = malloc(sizeof(connection_socket_ctx));
			if (ctx == NULL) {
				pthread_mutex_lock(&mtx_print);
				print_error("Failed to allocate memory for thread argument\n");
				pthread_mutex_unlock(&mtx_print);
				continue;
			}
			ctx->client_socket_fd = clientSocket;
			ctx->client_address = clientAddr;
			client_connection->ctx = ctx;
		} else {
			*client_connection = create_connection_SSL();
			connection_SSL_ctx* ctx;
			ctx = malloc(sizeof(connection_SSL_ctx));
			if (ctx == NULL) {
				continue;
			}
			ctx->client_socket_fd = clientSocket;
			ctx->client_address = clientAddr;
			ctx->ctx = server_SSL_ctx;
			client_connection->ctx = ctx;
		}
		if (pthread_create(&pthread, &pthread_attr, handle_client, (void*)client_connection) != 0) {
			pthread_mutex_lock(&mtx_print);
			print_error("Failed to create serve thread for client\n");
			pthread_mutex_unlock(&mtx_print);
			client_connection->close(client_connection->ctx);
			continue;
			
		}
	}
	closesocket(*serverSocket);
	free(serverSocket);
#ifdef _WIN32
	WSACleanup();
#endif
	free(ipAddressStr);

	if (SSL_ENCRYPT) {
		SSL_CTX_free(server_SSL_ctx);
		OPENSSL_cleanup();
	}

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
void send_json(connection_t* connection, const char* type, const char* data) {
	cJSON* root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "type", type);
	cJSON_AddStringToObject(root, "data", data);
	char* jsonStr = cJSON_PrintUnformatted(root);
	if (jsonStr) {
		uint32_t message_length = (uint32_t)strlen(jsonStr);
		uint32_t message_length_net = htonl(message_length);
		connection->write(connection->ctx, (char*)&message_length_net, sizeof(message_length_net));
		connection->write(connection->ctx, jsonStr, message_length);
		free(jsonStr);
	}
	cJSON_Delete(root);
}




// Receive message length
int receive_message_length(connection_t* connection, uint32_t* length)
{
	if (length == NULL || !connection || !connection->ctx || !connection->read)
		return EXIT_FAILURE;

	int received = 0, total_received = 0;
	while (total_received != sizeof(uint32_t))
	{
		received = connection->read(connection->ctx,((uint8_t*)length + total_received), sizeof(uint32_t) - total_received);
		if (received <= 0)
		{
			pthread_mutex_lock(&mtx_print);
			if (received == 0)
				print_error("Client disconnected before sending length\n");
			else
				print_error("recv error while reading length\n");
			pthread_mutex_unlock(&mtx_print);

			return EXIT_FAILURE;
		}
		total_received += received;
	}

	*length = ntohl(*length);

	return EXIT_SUCCESS;
}




/* Client service thread */
void* handle_client(void* arg) {
	connection_t* connection = (connection_t*)(arg);
	if (connection->init(connection->ctx)) {
		return NULL;
	}
	
	

	uint32_t message_length = 0;
	if (receive_message_length(connection, &message_length)) {
		connection->close(connection->ctx);
		return NULL;
	}

	if (message_length == 0 || message_length > MAX_MESSAGE_SIZE)
	{
		pthread_mutex_lock(&mtx_print);
		print_error("Invalid message length: %u\n", message_length);
		pthread_mutex_unlock(&mtx_print);
		connection->close(connection->ctx);
		return NULL;
	}


	char* json_buffer = (char*)malloc(message_length+1);
	if (!json_buffer)
	{
		pthread_mutex_lock(&mtx_print);
		print_error("Memory allocation error\n");
		pthread_mutex_unlock(&mtx_print);
		connection->close(connection->ctx);
		return NULL;
	}
	json_buffer[message_length] = '\0';


	int total_received = 0;
	while (total_received < message_length)
	{
		int n = 0;
		n = connection->read(connection->ctx, json_buffer + total_received, message_length - total_received);
		
		if (n <= 0)
			break;
		total_received += n;
	}
	if (total_received != message_length) {
		pthread_mutex_lock(&mtx_print);
		print_error("Failed to receive complete message\n");
		pthread_mutex_unlock(&mtx_print);
		free(json_buffer);
		connection->close(connection->ctx);
		return NULL;
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
			pthread_mutex_lock(&mtx_print);
			printf("[%s]: %s\n", typeItem->valuestring, dataItem->valuestring);
			pthread_mutex_unlock(&mtx_print);
			send_json(connection, "message", "OK");
		}
		else if (cJSON_IsString(typeItem) && cJSON_IsString(dataItem) && cJSON_IsString(saveToPath))
		{
			const char *type = typeItem->valuestring;
			const char *data = dataItem->valuestring;
			if (strcmp(type, "RAW_DATA") == 0) {
				pthread_mutex_lock(&mtx_print);
				printf("[%s]: %s\n", typeItem->valuestring, dataItem->valuestring);
				pthread_mutex_unlock(&mtx_print);
			}



			char safe_name[MAX_FILENAME + 1];
			sanitize_filename(safe_name, sizeof(safe_name), saveToPath->valuestring);


			pthread_mutex_lock(&mtx_fprint);
#ifdef _WIN32
			_mkdir(SAVE_DIR);
#else
			mkdir(SAVE_DIR, 0755);
#endif
			pthread_mutex_unlock(&mtx_fprint);


			int max_length = MAX_FILENAME + SAVE_DIR_LENGTH + 1; // <save_directory>/<file_name>
			char full_path[max_length];
			snprintf(full_path, sizeof(full_path), "%s/%s", SAVE_DIR, safe_name);

			pthread_mutex_lock(&mtx_fprint);
			FILE* file = fopen(full_path, "wb");
			if (!file)
			{
				pthread_mutex_unlock(&mtx_fprint);
				pthread_mutex_lock(&mtx_print);
				print_error("Failed to open %s\n", full_path);
				pthread_mutex_unlock(&mtx_print);
	
				send_json(connection, "error", "Cannot save file");
			}
			else
			{
				if (strcmp(type, "RAW_DATA") == 0)
				{
					fputs(data, file);
					fclose(file);
					pthread_mutex_unlock(&mtx_fprint);

					pthread_mutex_lock(&mtx_print);
					printf("[%s] saved to %s\n", type, full_path);
					pthread_mutex_unlock(&mtx_print);

					send_json(connection, "message", "File saved");
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
							pthread_mutex_unlock(&mtx_fprint);

							pthread_mutex_lock(&mtx_print);
							printf("[%s] saved to binary file \'%s\', %d bytes\n", type, full_path, decoded_len);
							pthread_mutex_unlock(&mtx_print);
							send_json(connection, "message", "File saved");
						}
						else
						{
							fclose(file);
							pthread_mutex_unlock(&mtx_fprint);

							pthread_mutex_lock(&mtx_print);
							print_error("Base64 decode failed\n");
							pthread_mutex_unlock(&mtx_print);
							send_json(connection, "error", "Base64 decode failed");
						}
						free(binary);
					}
					else
					{
						fclose(file);
						pthread_mutex_unlock(&mtx_fprint);

						pthread_mutex_lock(&mtx_print);
						print_error("Memory allocation error\n");
						pthread_mutex_unlock(&mtx_print);
						send_json(connection, "error", "Memory error");
					}
				}
				else
				{
					// Unknown type. Save the file
					fputs(data, file);
					fclose(file);
					pthread_mutex_unlock(&mtx_fprint);

					pthread_mutex_lock(&mtx_print);
					printf("[%s] saved to %s\n", type, full_path);
					pthread_mutex_unlock(&mtx_print);
					send_json(connection, "message", "File saved");
				}
			}
		}
		else
		{
			pthread_mutex_lock(&mtx_print);
			print_error("JSON missing 'type' or 'data' field\n");
			pthread_mutex_unlock(&mtx_print);
			send_json(connection, "error", "Invalid format");
		}
		cJSON_Delete(root);
	}
	else
	{
		pthread_mutex_lock(&mtx_print);
		print_error("Failed to parse JSON\n");
		pthread_mutex_unlock(&mtx_print);
		
		send_json(connection, "error", "JSON parse error");
	}
	// Cleanup
	free(json_buffer);

	connection->close(connection->ctx);

	return NULL;
}



/* Signal handler */
int set_up_signals_handling() {
	struct sigaction sa;
	memset((void*)&sa, 0, sizeof(sa));

	sa.sa_flags = 0; // Without SA_RESTART what	used to restart syscalls after error
	sa.sa_handler = signal_handler;
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGPIPE, &(struct sigaction){SIG_IGN}, NULL) == -1) {
		return EXIT_FAILURE;
	}
	if (sigaction(SIGTERM, &sa, NULL) == -1) {
		return EXIT_FAILURE;
	}
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
void signal_handler(int signal_number) {
	if (signal_number == SIGTERM || signal_number == SIGINT)
		keep_running = 0;
}



