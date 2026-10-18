/*
 * Server.c
 * Written by ClassZak
 * */
#include "util/config.h"
#include "util/get_global_ip.h"
#include "PrintProcedures/PrintProcedures.h"
#include "FileService/FileService.h"
#include "handlers/api_send.h"
#include "handlers/api_upload.h"

#include "civetweb.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

pthread_mutex_t mtx_print  = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx_fprint = PTHREAD_MUTEX_INITIALIZER;
volatile sig_atomic_t keep_running = 1;

static void on_signal(int s) { (void)s; keep_running = 0; }

static void install_signals(void) {
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = on_signal;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGINT,  &sa, NULL) == -1 ||
		sigaction(SIGTERM, &sa, NULL) == -1) {
		print_error(FAILED_SIGNAL_HANDLERS_MESSAGE);
		exit(EXIT_FAILURE);
	}
	signal(SIGPIPE, SIG_IGN);
}

int main(void) {
	print_pwd();
	install_signals();

	/* info-only: external IP */
	char *external = get_global_ip();
	if (external) {
		print_info("Server external IP: %s\n", external);
		free(external);
	}

	char ports[64];
	if (SSL_ENCRYPT) snprintf(ports, sizeof(ports), "%d,%ds", CONNECTION_PORT, TLS_PORT);
	else			 snprintf(ports, sizeof(ports), "%d",	 CONNECTION_PORT);

	const char *options[] = {
		"listening_ports",	 ports,
		"num_threads",		 "8",
		"request_timeout_ms",  "30000",
		"enable_keep_alive",   "yes",
		SSL_ENCRYPT ? "ssl_certificate" : "document_root",
		SSL_ENCRYPT ? PUBLIC_KEY_FILE   : ".",
		NULL
	};

	mg_init_library(0);

	struct mg_callbacks cb;
	memset(&cb, 0, sizeof(cb));
	struct mg_context *ctx = mg_start(&cb, NULL, options);
	if (!ctx) {
		print_error("mg_start failed\n");
		mg_exit_library();
		return EXIT_FAILURE;
	}

	mg_set_request_handler(ctx, "/api/send",   api_send_handler,   NULL);
	mg_set_request_handler(ctx, "/api/upload", api_upload_handler, NULL);

	print_success("Server started\n");
	print_info("  POST http://0.0.0.0:%d/api/send\n",   CONNECTION_PORT);
	print_info("  POST http://0.0.0.0:%d/api/upload\n", CONNECTION_PORT);
	print_info("  WS   ws://0.0.0.0:%d/api/stream\n",   CONNECTION_PORT);
	if (SSL_ENCRYPT)
		print_info("  HTTPS/WSS on port %d (cert: %s)\n", TLS_PORT, PUBLIC_KEY_FILE);

	while (keep_running) sleep(1);

	print_info("Stopping...\n");
	mg_stop(ctx);
	mg_exit_library();
	return EXIT_SUCCESS;
}

