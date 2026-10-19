/*
 * Server.c
 * Written by ClassZak
 * */
#include "util/config.h"
#include "util/get_server_ip_addresses.h"
#include "util/get_public_ip_upnp.h"
#include "util/get_public_ip_http.h"
#include "PrintProcedures/PrintProcedures.h"
#include "FileService/FileService.h"
#include "handlers/api_send.h"
#include "handlers/api_upload.h"


#include "civetweb.h"


#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>




pthread_mutex_t mtx_print  = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx_fprint = PTHREAD_MUTEX_INITIALIZER;
volatile sig_atomic_t keep_running = 1;




static void on_signal(int s) {
	(void)s;
	keep_running = 0;
}




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




long get_cpu_count() {
	long nprocs;
#ifndef _WIN32
	nprocs = sysconf(_SC_NPROCESSORS_ONLN);
	if (nprocs == -1) {
		print_error("Failed to get CPU count");
		int err = errno;
		print_error("Error %d.\t%s\n", err, strerror(err));
	}
#else
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	nprocs = sysInfo.dwNumberOfProcessors;
#endif

	return nprocs;
}




static inline void print_server_ip_addresses(){
	int		ip_addresses_count;
	char**	ip_addresses = get_server_ip_addresses(&ip_addresses_count, SERVER_IP_WITH_MASK);
	if (ip_addresses_count == 0) {
		print_error("Failed to receive ip addresess\n");
	} else {
		print_info("Aviable IP addresses:\n");
		for (int i = 0; i != ip_addresses_count; ++i) {
			print_info("%s\n", ip_addresses[i]);
		}
		free_server_ip_addresses(ip_addresses, ip_addresses_count);
	}
}




/**
 * @brief Worker thread: logs all local addresses, then tries to
 *        discover the public IP (UPnP first, HTTP as fallback).
 *
 * Runs in the background so a slow HTTP lookup (up to 5 s) does not
 * delay server startup.
 */
static void* log_addresses_worker(void* arg) {
	(void)arg;

	/* 1. Local addresses */
	int n = 0;
	char** ips = get_server_ip_addresses(&n, SERVER_IP_WITH_MASK);
	if (ips != NULL) {
		print_info("Local IP addresses (%d):\n", n);
		for (int i = 0; i < n; i++) {
			print_info("  %s\n", ips[i]);
		}
		free_server_ip_addresses(ips, n);
	} else {
		print_info("Local IP addresses: none found\n");
	}

	/* 2. Public IP — UPnP first */
	char* public_ip = get_public_ip_upnp();
	if (public_ip != NULL) {
		print_info("Public IP (UPnP): %s\n", public_ip);
		free(public_ip);

		return NULL;
	}

	/* 3. Public IP — HTTP fallback */
	public_ip = get_public_ip_http();
	if (public_ip != NULL) {
		print_info("Public IP (HTTP): %s\n", public_ip);
		free(public_ip);
	} else {
		print_info("Public IP: unknown (UPnP and HTTP both failed)\n");
	}

	return NULL;
}















int main(void) {
	print_pwd();
	install_signals();

	mg_init_library(0);

	/* info-only: external IP */
	print_server_ip_addresses();

	pthread_t addr_thread;
	if (pthread_create(&addr_thread, NULL, log_addresses_worker,NULL) == 0) {
		pthread_detach(addr_thread);
	}

	char ports[64];
	if (SSL_ENCRYPT) {
		print_info("Set up SSL encryption\n");
		// HTTPS only, same port number
		snprintf(ports, sizeof(ports), "%ds", CONNECTION_PORT);
	} else {
		// HTTP only
		snprintf(ports, sizeof(ports), "%d", CONNECTION_PORT);
	}

	char cpu_count[23];
	snprintf(cpu_count, sizeof(cpu_count), "%ld", get_cpu_count());
	const char* options[] = {
		"listening_ports",	 ports,
		"num_threads",		 cpu_count,
		"request_timeout_ms",  "30000",
		"enable_keep_alive",   "yes",
		SSL_ENCRYPT ? "ssl_certificate" : "document_root",
		SSL_ENCRYPT ? PUBLIC_KEY_FILE   : ".",
		NULL
	};


	struct mg_callbacks cb;
	memset(&cb, 0, sizeof(cb));
	struct mg_context* ctx = mg_start(&cb, NULL, options);
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
	if (SSL_ENCRYPT)
		print_info("  HTTPS/WSS on port %d (cert: %s)\n", CONNECTION_PORT, PUBLIC_KEY_FILE);

	while (keep_running) sleep(1);

	print_info("Stopping...\n");
	mg_stop(ctx);
	mg_exit_library();
	return EXIT_SUCCESS;
}

