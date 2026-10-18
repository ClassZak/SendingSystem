/*
 * Server.c
 * Written by ClassZak
 * */

#include "util/config.h"
#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#endif




#include "PrintProcedures/PrintProcedures.h"
#include "FileService/FileService.h"
#include "Base64/Base64.h"
#include "OpenSSL/OpenSSL.h"


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
#include <sqlite3.h>




// ---------- Constants and macros ----------
#include "util/config.h"
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
#include "util/set_socket_timeout.h"
#include "util/get_global_ip.h"
#include "util/setup_socket.h"
#include "util/receive_message_length.h"
void send_json(connection_t* connection, const char* type, const char* data);



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






// ---------- Send JSON response ----------
/*
 * Response types:
 *	1) error
 *	2) message
 * */
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




/* Client service thread */
void* handle_client(void* arg) {
	connection_t* connection = (connection_t*)(arg);
	if (connection->init(connection->ctx)) {
		return NULL;
	}
	
	

	uint32_t message_length = 0;
	if (receive_message_length(connection, &message_length, &mtx_print)) {
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

