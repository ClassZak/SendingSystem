#include "http_json.h"
#include <cJSON.h>
#include <string.h>
#include <stdlib.h>

void http_send_json(struct mg_connection *conn, int status,
					const char *type, const char *data) {
	cJSON *root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "type", type);
	cJSON_AddStringToObject(root, "data", data);
	char *json = cJSON_PrintUnformatted(root);
	int len = json ? (int)strlen(json) : 0;

	const char *reason = (status == 200) ? "OK"
					   : (status == 400) ? "Bad Request"
					   : (status == 405) ? "Method Not Allowed"
					   : (status == 500) ? "Internal Server Error"
					   : "Error";

	mg_printf(conn,
			  "HTTP/1.1 %d %s\r\n"
			  "Content-Type: application/json\r\n"
			  "Content-Length: %d\r\n"
			  "Connection: close\r\n\r\n",
			  status, reason, len);
	if (len > 0) mg_write(conn, json, len);

	free(json);
	cJSON_Delete(root);
}

void http_send_error(struct mg_connection *conn, int status, const char *msg) {
	http_send_json(conn, status, "error", msg);
}

