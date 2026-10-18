#include "get_global_ip.h"
#include "config.h"
#include "../PrintProcedures/PrintProcedures.h"

#include <civetweb.h>	 /* сам подтянет WinSock / sys/socket.h */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IP_SOLVER_HOST   "icanhazip.com"
#define IP_SOLVER_PORT   80
#define IP_RESPONSE_BUF  4096

char* get_global_ip(void) {
	char errbuf[256];
	struct mg_connection *c = mg_connect_client(
		IP_SOLVER_HOST, IP_SOLVER_PORT, 0, errbuf, sizeof(errbuf));
	if (!c) {
		print_error("get_global_ip: connect failed: %s\n", errbuf);
		return NULL;
	}

	mg_printf(c,
			  "GET / HTTP/1.1\r\n"
			  "Host: %s\r\n"
			  "Connection: close\r\n"
			  "\r\n",
			  IP_SOLVER_HOST);

	char response[IP_RESPONSE_BUF];
	size_t total = 0;
	int n;
	while (total < sizeof(response) - 1) {
		n = mg_read(c, response + total, sizeof(response) - 1 - total);
		if (n <= 0) break;
		total += (size_t)n;
	}
	mg_close_connection(c);
	response[total] = '\0';

	if (total == 0) {
		print_error("get_global_ip: empty response\n");
		return NULL;
	}

	/* Отделяем тело от заголовков */
	char *body = strstr(response, "\r\n\r\n");
	if (!body) {
		print_error("get_global_ip: malformed response\n");
		return NULL;
	}
	body += 4;

	/* Обрезаем по первому пробельному символу */
	size_t len = strcspn(body, "\r\n\t ");
	if (len == 0) {
		print_error("get_global_ip: empty body\n");
		return NULL;
	}

	char *ip = malloc(len + 1);
	if (!ip) return NULL;
	memcpy(ip, body, len);
	ip[len] = '\0';
	return ip;
}

