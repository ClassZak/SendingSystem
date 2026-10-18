#include "get_global_ip.h"
#include "config.h"
#include "set_socket_timeout.h"


#include "../PrintProcedures/PrintProcedures.h"




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

