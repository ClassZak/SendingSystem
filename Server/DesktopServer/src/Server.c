/*
 * Sending System
 * Copyright (C) 2025-2026 ClassZak
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
/**
 * Entry point of the HTTP/HTTPS server.
 *
 * Exposes two endpoints:
 *   - POST /api/send    — text messages (JSON)
 *   - POST /api/upload  — file uploads (multipart/form-data)
 *
 * Startup: install signals → init CivetWeb → spawn IP-logging thread
 * → open database → start CivetWeb → register handlers → block until
 * SIGINT/SIGTERM.
 */

#include "util/config.h"
#include "util/get_server_ip_addresses.h"
#include "util/get_public_ip_upnp.h"
#include "util/get_public_ip_http.h"
#include "PrintProcedures/PrintProcedures.h"
#include "FileService/FileService.h"
#include "handlers/api_send.h"
#include "handlers/api_upload.h"
#include "database/Database.h"
#include "database/database_config.h"


#include "civetweb.h"


#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>




/** Serialises all writes to stdout/stderr from worker threads. */
pthread_mutex_t mtx_print  = PTHREAD_MUTEX_INITIALIZER;
/** Serialises all writes to the file system. */
pthread_mutex_t mtx_fprint = PTHREAD_MUTEX_INITIALIZER;
/** Set to 0 by the signal handler to stop the accept loop. */
volatile sig_atomic_t keep_running = 1;




/**
 * Signal handler for SIGINT and SIGTERM.
 *
 * Sets keep_running to 0, which stops the main loop and triggers a
 * graceful shutdown.
 *
 * @param signal_number (int, Required) — signal number (unused).
 */
static void on_signal(int signal_number) {
	keep_running = 0;
}




/**
 * Installs SIGINT/SIGTERM handlers and ignores SIGPIPE.
 *
 * Without SIGPIPE being ignored, a write to a disconnected client
 * would terminate the entire process.
 */
static void install_signals(void) {
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = on_signal;
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGINT, &sa, NULL) == -1 ||
	    sigaction(SIGTERM, &sa, NULL) == -1) {
		print_error(FAILED_SIGNAL_HANDLERS_MESSAGE);
		exit(EXIT_FAILURE);
	}

	signal(SIGPIPE, SIG_IGN);
}




/**
 * Returns the number of online processors, used to size the CivetWeb
 * thread pool.
 *
 * @return (long) — number of CPUs; -1 on sysconf failure (Linux only).
 */
static long get_cpu_count(void) {
	long nprocs;

#ifndef _WIN32
	nprocs = sysconf(_SC_NPROCESSORS_ONLN);
	if (nprocs == -1) {
		int err = errno;
		print_error("Failed to get CPU count: %d, %s\n", err, strerror(err));
	}
#else
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);
	nprocs = sys_info.dwNumberOfProcessors;
#endif

	return nprocs;
}




/**
 * Background worker: prints every local IP address and attempts to
 * discover the public IP (UPnP first, HTTP fallback).
 *
 * Runs in its own detached thread so that a slow HTTP lookup (up to
 * 5 seconds) does not delay server startup.
 *
 * @param arg (void*, Optional) — unused.
 * @return (void*) — always NULL.
 */
static void* log_addresses_worker(void* arg) {
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


	char* public_ip = get_public_ip_upnp();
	if (public_ip != NULL) {
		print_info("Public IP (UPnP): %s\n", public_ip);
		free(public_ip);

		return NULL;
	}


	public_ip = get_public_ip_http();
	if (public_ip != NULL) {
		print_info("Public IP (HTTP): %s\n", public_ip);
		free(public_ip);
	} else {
		print_info("Public IP: unknown (UPnP and HTTP both failed)\n");
	}

	return NULL;
}




/**
 * Program entry point.
 *
 * @return (int) — EXIT_SUCCESS on clean shutdown, EXIT_FAILURE if the
 *         database or the HTTP server cannot be started.
 */
int main(void) {
	install_signals();
	print_pwd();


	mg_init_library(0);

	pthread_t addr_thread;
	if (pthread_create(&addr_thread, NULL, log_addresses_worker, NULL) == 0) {
		pthread_detach(addr_thread);
	}


	sqlite3* db = NULL;
	if (database_open(DATABASE_FILENAME, &db) != 0) {
		print_error("Failed to open database\n");
		mg_exit_library();

		return EXIT_FAILURE;
	}


	char ports[64];
	if (SSL_ENCRYPT) {
		snprintf(ports, sizeof(ports), "%ds", CONNECTION_PORT);
	} else {
		snprintf(ports, sizeof(ports), "%d", CONNECTION_PORT);
	}

	char cpu_count_buf[23];
	snprintf(cpu_count_buf, sizeof(cpu_count_buf), "%ld", get_cpu_count());

	const char* options[] = {
		"listening_ports",     ports,
		"num_threads",         cpu_count_buf,
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
		database_close(db);
		mg_exit_library();

		return EXIT_FAILURE;
	}


	mg_set_request_handler(ctx, "/api/send",   api_send_handler,   NULL);
	mg_set_request_handler(ctx, "/api/upload", api_upload_handler, NULL);

	print_success("Server started\n");
	print_info("  POST http://0.0.0.0:%d/api/send\n",   CONNECTION_PORT);
	print_info("  POST http://0.0.0.0:%d/api/upload\n", CONNECTION_PORT);
	if (SSL_ENCRYPT) {
		print_info("  HTTPS on port %d (cert: %s)\n", CONNECTION_PORT, PUBLIC_KEY_FILE);
	}


	while (keep_running) {
		sleep(1);
	}

	print_info("Stopping...\n");
	mg_stop(ctx);
	database_close(db);
	mg_exit_library();

	return EXIT_SUCCESS;
}

