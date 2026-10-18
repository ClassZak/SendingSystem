#include "api_upload.h"
#include "http_json.h"
#include "../util/config.h"
#include "../FileService/FileService.h"
#include "../PrintProcedures/PrintProcedures.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

extern pthread_mutex_t mtx_print;
extern pthread_mutex_t mtx_fprint;

typedef struct {
	FILE	 *fp;
	char	  path[MAX_FILENAME + SAVE_DIR_LENGTH + 2];
	long long size;
} upload_ctx_t;

static int on_field_found(const char *key, const char *filename,
						  char *path, size_t pathlen, void *user_data) {
	(void)key;
	upload_ctx_t *u = user_data;

	if (!filename || !filename[0]) return MG_FORM_FIELD_STORAGE_GET;

	char safe_name[MAX_FILENAME + 1];
	sanitize_filename(safe_name, sizeof(safe_name), filename);

#ifdef _WIN32
	_mkdir(SAVE_DIR);
#else
	mkdir(SAVE_DIR, 0755);
#endif

	snprintf(path, pathlen, "%s/%s", SAVE_DIR, safe_name);
	snprintf(u->path, sizeof(u->path), "%s", path);
	u->size = 0;

	pthread_mutex_lock(&mtx_fprint);
	u->fp = fopen(path, "wb");
	pthread_mutex_unlock(&mtx_fprint);

	if (!u->fp) {
		print_error("Cannot open %s for write\n", path);
		return MG_FORM_FIELD_STORAGE_ABORT;
	}
	print_info("[UPLOAD] Receiving %s\n", path);
	return MG_FORM_FIELD_STORAGE_STORE;
}

static int on_field_get(const char *key, const char *value,
						size_t valuelen, void *user_data) {
	(void)key; (void)value; (void)valuelen; (void)user_data;
	return 0;
}

static int on_field_store(const char *path, long long size, void *user_data) {
	upload_ctx_t *u = user_data;
	if (u->fp) {
		pthread_mutex_lock(&mtx_fprint);
		fclose(u->fp);
		pthread_mutex_unlock(&mtx_fprint);
		u->fp = NULL;
	}
	print_info("[UPLOAD] Stored %s (%lld bytes)\n", path, size);
	return 0;
}

int api_upload_handler(struct mg_connection *conn, void *cbdata) {
	(void)cbdata;
	const struct mg_request_info *ri = mg_get_request_info(conn);
	if (strcmp(ri->request_method, "POST") != 0) {
		http_send_error(conn, 405, "POST required");
		return 1;
	}

	upload_ctx_t u = {0};
	struct mg_form_data_handler fdh = {0};
	fdh.field_found = on_field_found;
	fdh.field_get   = on_field_get;
	fdh.field_store = on_field_store;
	fdh.user_data   = &u;

	int fields = mg_handle_form_request(conn, &fdh);
	if (u.fp) { fclose(u.fp); u.fp = NULL; }

	if (fields < 0) http_send_error(conn, 400, "Form parse error");
	else			http_send_json (conn, 200, "message", "File(s) received");

	return 1;
}

