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
#include "Server.h"
#include "Client.h"
#include "database/model/database_enums.h"

#include "PrintProcedures/PrintProcedures.h"

#include <libterm/libterm.h>


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>








typedef enum {
	LAUNCH_TYPE_DEFAULT = 1,
	LAUNCH_TYPE_ERROR   = 2,
	LAUNCH_TYPE_HELP    = 3,
	LAUNCH_TYPE_VERSION = 4
} LaunchType;


typedef struct {
	LaunchType       launch_type;
	uint16_t         connection_port;
	char*            connection_ip;
	ConnectionRoleId incoming_peer_role;
	bool             ip_set;
	bool             port_set;
	bool             role_set;
} ServerConfig;


typedef enum {
	KIND_FLAG,
	KIND_PORT,
	KIND_IP,
	KIND_ROLE
} value_kind;


typedef enum {
	ACT_NONE = 0,
	ACT_HELP,
	ACT_VERSION
} option_action;


typedef struct {
	const char*   short_flag;
	const char*   long_flag;
	const char*   description;
	value_kind    kind;
	size_t        offset;
	option_action act;
} ArgumentMapping;




/**
 * Prints the final server configuration via printf.
 *
 * A NULL connection_ip field is printed as "(not set)"; the role is duplicated
 * with a word (server/client) for log readability; launch_type — as a string.
 * The ip_set/port_set/role_set flags are not printed: they are internal
 * parser markers, not part of the configuration.
 *
 * @param cfg (const ServerConfig*, Required) — configuration to print.
 *        Expected to be initialized via parse_commands and (if
 *        necessary) interactive_config.
 */
void print_server_config(const ServerConfig* cfg);




/**
 * Unset ServerConfig fields.
 *
 * @param cfg (const ServerConfig*, Required) — configuration to unset
 */
void unset_server_config(ServerConfig* cfg);




/**
 * Prints help for all options from the options table.
 *
 * @param app_name (const char*, Required) — application name for the header
 *        and the Usage line.
 */
void print_help(const char* app_name);


/**
 * Prints the version string.
 *
 * @param app_name (const char*, Required) — application name.
 */
void print_version(const char* app_name);


/**
 * Searches for an option in the table by short or long flag.
 *
 * @param arg (const char*, Required) — argument (e.g., "-h" or "--help").
 * @return (int) — index in options; -1 if not found.
 */
static int find_option(const char* arg);


/**
 * Parses an integer from a string with range checking [min, max].
 *
 * @param s   (const char*, Required) — input string.
 * @param min (long, Required) — minimum allowed value (inclusive).
 * @param max (long, Required) — maximum allowed value (inclusive).
 * @param out (long*, Required) — where to write the result on success.
 * @return (int) — 0 on success; -1 on parse error or out of range.
 */
static int parse_int_in_range(const char* s, long min, long max, long* out);


/**
 * Prompts for missing configuration fields via libterm.
 *
 * Checks the ip_set / port_set / role_set flags; if all three are set,
 * the UI is not initialized. Otherwise, it brings up libterm and sequentially
 * prompts for IP, port, and role for the missing fields.
 *
 * @param cfg (ServerConfig*, Required) — configuration; the ip_set/port_set/
 *        role_set fields are used as input (what is already set) and output
 *        (what is filled interactively).
 * @return (int) — EXIT_SUCCESS (0) if all fields are filled;
 *        EXIT_FAILURE (1) on UI initialization, input, or memory error.
 */
static int interactive_config(ServerConfig* cfg);


/**
 * Reads a string from the keyboard, drawing the label and current value
 * on line y. Enter completes input, Backspace deletes the last character.
 * Only printable ASCII (32..126) are allowed. Maximum (max_len - 1) characters
 * plus a terminating '\0'. The label and value are redrawn on each step —
 * the behavior does not depend on whether the library preserves the previous frame.
 *
 * @param y       (int, Required) — screen line.
 * @param label   (const char*, Required) — label (e.g., "IP address: ").
 * @param buf     (char*, Required) — output buffer; on success contains a '\0'-
 *        terminated entered string.
 * @param max_len (size_t, Required) — size of buf in bytes (including '\0').
 * @return (int) — 0 on Enter; -1 on UI close or poll error.
 */
static int prompt_text(int y, const char* label, char* buf, size_t max_len);


/**
 * Prompts for a port until a valid value 1..UINT16_MAX is obtained.
 *
 * @param y        (int, Required) — screen line.
 * @param out_port (uint16_t*, Required) — where to write the result.
 * @return (int) — 0 on success; -1 on input error.
 */
static int prompt_port(int y, uint16_t* out_port);


/**
 * Prompts for a role (0 = SERVER, 1 = CLIENT) until a valid value is obtained.
 *
 * @param y        (int, Required) — screen line.
 * @param out_role (ConnectionRoleId*, Required) — where to write the result.
 * @return (int) — 0 on success; -1 on input error.
 */
static int prompt_role(int y, ConnectionRoleId* out_role);


/**
 * Parses argv into ServerConfig.
 *
 * A repeated option, unknown option, or invalid value
 * results in returning EXIT_FAILURE (code 1). The -h/--help and
 * -v/--version options only set launch_type; the final decision
 * on printing and exit code is made by main.
 *
 * @param argc (int, Required) — number of arguments.
 * @param argv (char**, Required) — array of arguments (argv[0] is the program name).
 * @param cfg  (ServerConfig*, Required) — output structure; initialized
 *        with default values; the ip_set/port_set/role_set flags reflect
 *        whether the corresponding fields were explicitly set.
 * @return (int) — EXIT_SUCCESS (0) or EXIT_FAILURE (1).
 */
int parse_commands(int argc, char** argv, ServerConfig* cfg);




int main(int argc, char** argv) {
	ServerConfig server_config;
	if (parse_commands(argc, argv, &server_config) != EXIT_SUCCESS) {
		print_help(PROJECT_NAME);
		return EXIT_FAILURE;
	}

	switch (server_config.launch_type) {
	case LAUNCH_TYPE_HELP:
		print_help(PROJECT_NAME);
		return EXIT_SUCCESS;
	case LAUNCH_TYPE_VERSION:
		print_version(PROJECT_NAME);
		return EXIT_SUCCESS;
	default:
		break;
	}

	do {
		if (interactive_config(&server_config) != EXIT_SUCCESS)
			return EXIT_FAILURE;

		print_server_config(&server_config);

		if (server() == EXIT_FAILURE) {
			return EXIT_FAILURE;
		}


		bool do_exit = false;
		if (lt_init() != LT_OK) {
			print_error("Failed libterm initialization\n");
			print_error_log("Error in file '%s', line:%d in '%s'\n", __FILE__, __LINE__, __FUNCTION__);
			return EXIT_FAILURE;
		}
		lt_print(1, 1, LT_GREEN, LT_DEFAULT, "'q' or 'Esc' key to quit. Another key to reset configuration");

		struct lt_event ev;
		while (lt_poll_event(&ev) == LT_OK) {
			if (ev.type != LT_EVENT_KEY)
				continue;
			if (ev.ch == 'q' || ev.key == LT_KEY_ESC)
				do_exit = true;
			else
				do_exit = false;
		}


		lt_shutdown();
		unset_server_config(&server_config);
	} while(1);

	return EXIT_SUCCESS;
}




void print_server_config(const ServerConfig* cfg) {
	if (cfg == NULL) {
		print_error("print_server_config: cfg is NULL\n");
		print_error("Error in file '%s', line:%d in '%s'\n",
		            __FILE__, __LINE__, __FUNCTION__);

		return;
	}

	const char* role_name;
	switch (cfg->incoming_peer_role) {
	case CONNECTION_ROLE_CLIENT:
		role_name = "client";
		break;
	case CONNECTION_ROLE_SERVER:
		role_name = "server";
		break;
	default:
		role_name = "unknown";
		break;
	}

	const char* launch_name;
	switch (cfg->launch_type) {
	case LAUNCH_TYPE_HELP:
		launch_name = "help";
		break;
	case LAUNCH_TYPE_VERSION:
		launch_name = "version";
		break;
	case LAUNCH_TYPE_ERROR:
		launch_name = "error";
		break;
	default:
		launch_name = "default";
		break;
	}

	printf("Server configuration:\n");
	printf("  connection_ip      : %s\n",
	           cfg->connection_ip ? cfg->connection_ip : "(not set)");
	printf("  connection_port    : %u\n",
	           (unsigned)cfg->connection_port);
	printf("  incoming_peer_role : %d (%s)\n",
	           (int)cfg->incoming_peer_role, role_name);
	printf("  launch_type        : %s\n", launch_name);
}




void unset_server_config(ServerConfig* cfg) {
	cfg->ip_set = false;
	cfg->port_set = false;
	cfg->role_set = false;
}



static const ArgumentMapping options[] = {
	{"-h", "--help", "Show help and exit",
	 KIND_FLAG, 0, ACT_HELP},
	{"-v", "--version", "Show version and exit",
	 KIND_FLAG, 0, ACT_VERSION},
	{"-p", "--port", "Port number to bind or connect to",
	 KIND_PORT, offsetof(ServerConfig, connection_port), ACT_NONE},
	{"-i", "--ip", "IP address to bind or connect to",
	 KIND_IP, offsetof(ServerConfig, connection_ip), ACT_NONE},
	{"-c", "--incoming_peer_role",
	 "Role designation for incoming peer connections",
	 KIND_ROLE, offsetof(ServerConfig, incoming_peer_role), ACT_NONE}
};

enum { OPTIONS_COUNT = sizeof(options) / sizeof(options[0]) };




static int find_option(const char* arg) {
	for (size_t i = 0; i < OPTIONS_COUNT; ++i) {
		if ((options[i].short_flag[0] &&
		     strcmp(arg, options[i].short_flag) == 0) ||
		    strcmp(arg, options[i].long_flag) == 0) {
			return (int)i;
		}
	}

	return -1;
}


static int parse_int_in_range(const char* s, long min, long max, long* out) {
	char* end;
	long v = strtol(s, &end, 10);
	if (*end != '\0' || v < min || v > max)
		return -1;

	*out = v;

	return 0;
}




static int prompt_text(int y, const char* label, char* buf, size_t max_len) {
	size_t len = 0;
	int x = 1 + (int)strlen(label);
	int width = (int)(max_len - 1);
	char display[128];

	buf[0] = '\0';

	snprintf(display, sizeof(display), "%-*s", width, buf);
	lt_print(1, y, LT_GREEN, LT_DEFAULT, label);
	lt_print(x, y, LT_GREEN, LT_DEFAULT, display);
	lt_present();

	struct lt_event event;
	while (lt_poll_event(&event) == LT_OK) {
		if (event.type != LT_EVENT_KEY)
			continue;

		if (event.key == LT_KEY_ENTER ||
		    event.ch == '\r' || event.ch == '\n')
			return 0;

		if (event.key == LT_KEY_BACKSPACE || event.key == LT_KEY_BACKSPACE2) {
			if (len > 0)
				buf[--len] = '\0';
		} else if (event.ch >= ' ' && event.ch <= '~' &&
		           len + 1 < max_len) {
			buf[len++] = event.ch;
			buf[len] = '\0';
		} else {
			continue;
		}

		snprintf(display, sizeof(display), "%-*s", width, buf);
		lt_print(1, y, LT_GREEN, LT_DEFAULT, label);
		lt_print(x, y, LT_GREEN, LT_DEFAULT, display);
		lt_present();
	}

	return -1;
}


static int prompt_port(int y, uint16_t* out_port) {
	char buf[8];
	while (true) {
		lt_print(1, y + 1, LT_DEFAULT, LT_DEFAULT,
		         "                                                 ");
		lt_present();

		if (prompt_text(y, "Port (1-65535): ", buf, sizeof(buf)) != 0)
			return -1;

		long v;
		if (parse_int_in_range(buf, 1, UINT16_MAX, &v) == 0) {
			*out_port = (uint16_t)v;
			return 0;
		}

		lt_print(1, y + 1, LT_RED, LT_DEFAULT,
		         "Invalid port, try again                          ");
		lt_present();
	}
}


static int prompt_role(int y, ConnectionRoleId* out_role) {
	char buf[4];
	while (true) {
		lt_print(1, y + 1, LT_DEFAULT, LT_DEFAULT,
		         "                                                 ");
		lt_present();

		if (prompt_text(y,
		                "Incoming peer role (0=server, 1=client): ",
		                buf, sizeof(buf)) != 0)
			return -1;

		long v;
		if (parse_int_in_range(buf, 0, 1, &v) == 0) {
			*out_role = (v == 1) ? CONNECTION_ROLE_CLIENT
			                     : CONNECTION_ROLE_SERVER;
			return 0;
		}

		lt_print(1, y + 1, LT_RED, LT_DEFAULT,
		         "Invalid role, expected 0 or 1                    ");
		lt_present();
	}
}




static int interactive_config(ServerConfig* cfg) {
	if (cfg->ip_set && cfg->port_set && cfg->role_set)
		return EXIT_SUCCESS;

	int lt_err = lt_init();
	if (lt_err != LT_OK) {
		print_error("Error console UI initialization\n");
		print_error("Error in file '%s', line:%d in '%s'\n",
		            __FILE__, __LINE__, __FUNCTION__);

		return EXIT_FAILURE;
	}

	int y = 1;

	if (!cfg->ip_set) {
		char ip_buffer[64];
		if (prompt_text(y++, "IP address: ",
		                ip_buffer, sizeof(ip_buffer)) != 0)
			goto fail;

		size_t ip_length = strlen(ip_buffer);
		cfg->connection_ip = malloc(ip_length + 1);
		if (cfg->connection_ip == NULL) {
			print_error("Failed to allocate IP buffer\n");
			print_error("Error in file '%s', line:%d in '%s'\n",
			            __FILE__, __LINE__, __FUNCTION__);

			goto fail;
		}

		snprintf(cfg->connection_ip, ip_length + 1, "%s", ip_buffer);
		cfg->ip_set = true;
	}

	if (!cfg->port_set) {
		if (prompt_port(y++, &cfg->connection_port) != 0)
			goto fail;

		cfg->port_set = true;
	}

	if (!cfg->role_set) {
		if (prompt_role(y++, &cfg->incoming_peer_role) != 0)
			goto fail;

		cfg->role_set = true;
	}

	lt_shutdown();

	return EXIT_SUCCESS;

fail:
	lt_shutdown();

	return EXIT_FAILURE;
}




void print_help(const char* app_name) {
	printf("%s version 1.0.0\n\n", app_name);
	printf("Usage : %s [options] <arguments ...>\n\n", app_name);
	printf(" This is just a short option summary. "
	           "Please see \"man %s\" for details.\n\n", app_name);
	printf("OPTIONS\n");
	for (size_t i = 0; i < OPTIONS_COUNT; ++i) {
		char flag_buffer[64];
		if (options[i].short_flag[0] != '\0') {
			snprintf(flag_buffer, sizeof(flag_buffer), "%s, %s",
			         options[i].short_flag, options[i].long_flag);
		} else {
			snprintf(flag_buffer, sizeof(flag_buffer), "    %s",
			         options[i].long_flag);
		}

		printf(" %-26s %s\n", flag_buffer, options[i].description);
	}
}




void print_version(const char* app_name) {
	printf("%s version 1.0.0\n", app_name);
}




int parse_commands(int argc, char** argv, ServerConfig* cfg) {
	if (cfg == NULL)
		return EXIT_FAILURE;

	cfg->launch_type = LAUNCH_TYPE_DEFAULT;
	cfg->connection_port = 0;
	cfg->connection_ip = NULL;
	cfg->incoming_peer_role = CONNECTION_ROLE_SERVER;
	cfg->ip_set = false;
	cfg->port_set = false;
	cfg->role_set = false;

	bool seen[OPTIONS_COUNT];
	memset(seen, 0, sizeof(seen));

	for (int i = 1; i < argc; ++i) {
		const char* arg = argv[i];
		int idx = find_option(arg);
		if (idx < 0) {
			print_error("Unknown option: %s\n", arg);
			print_error("Error in file '%s', line:%d in '%s'\n",
			            __FILE__, __LINE__, __FUNCTION__);

			return EXIT_FAILURE;
		}

		if (seen[idx]) {
			print_error("Duplicate option: %s\n", arg);
			print_error("Error in file '%s', line:%d in '%s'\n",
			            __FILE__, __LINE__, __FUNCTION__);

			return EXIT_FAILURE;
		}

		seen[idx] = true;

		const ArgumentMapping* mapping = &options[idx];
		if (mapping->act == ACT_HELP) {
			cfg->launch_type = LAUNCH_TYPE_HELP;
			continue;
		}

		if (mapping->act == ACT_VERSION) {
			if (cfg->launch_type != LAUNCH_TYPE_HELP)
				cfg->launch_type = LAUNCH_TYPE_VERSION;

			continue;
		}

		switch (mapping->kind) {
		case KIND_FLAG:
			*(bool*)((char*)cfg + mapping->offset) = true;
			break;
		case KIND_PORT: {
			if (i + 1 >= argc) {
				print_error("Option %s requires a value\n", arg);
				print_error("Error in file '%s', line:%d in '%s'\n",
				            __FILE__, __LINE__, __FUNCTION__);

				return EXIT_FAILURE;
			}

			long v;
			if (parse_int_in_range(argv[++i], 0, UINT16_MAX, &v) != 0) {
				print_error("Invalid port value for %s\n", arg);
				print_error("Error in file '%s', line:%d in '%s'\n",
				            __FILE__, __LINE__, __FUNCTION__);

				return EXIT_FAILURE;
			}

			*(uint16_t*)((char*)cfg + mapping->offset) = (uint16_t)v;
			cfg->port_set = true;
			break;
		}
		case KIND_IP:
			if (i + 1 >= argc) {
				print_error("Option %s requires a value\n", arg);
				print_error("Error in file '%s', line:%d in '%s'\n",
				            __FILE__, __LINE__, __FUNCTION__);

				return EXIT_FAILURE;
			}

			*(char**)((char*)cfg + mapping->offset) = argv[++i];
			cfg->ip_set = true;
			break;
		case KIND_ROLE: {
			if (i + 1 >= argc) {
				print_error("Option %s requires a value\n", arg);
				print_error("Error in file '%s', line:%d in '%s'\n",
				            __FILE__, __LINE__, __FUNCTION__);

				return EXIT_FAILURE;
			}

			long v;
			if (parse_int_in_range(argv[++i], 0, 1, &v) != 0) {
				print_error("Invalid role for %s (0 or 1 expected)\n", arg);
				print_error("Error in file '%s', line:%d in '%s'\n",
				            __FILE__, __LINE__, __FUNCTION__);

				return EXIT_FAILURE;
			}

			*(ConnectionRoleId*)((char*)cfg + mapping->offset) =
				(v == 1) ? CONNECTION_ROLE_CLIENT : CONNECTION_ROLE_SERVER;
			cfg->role_set = true;
			break;
		}
		}
	}

	return EXIT_SUCCESS;
}

