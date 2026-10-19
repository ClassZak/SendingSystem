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
 * @file    get_server_ip_addresses.c
 * @brief   Enumerates every distinct subnet attached to the host and
 *          returns one IP address per subnet.
 *
 * ── Algorithm ─────────────────────────────────────────────────────────
 *
 * 1. Enumerate all interfaces and their IP addresses.
 * 2. For each address compute the subnet it belongs to:
 *        IPv4:  network = ip & mask,  prefix = popcount(mask)
 *        IPv6:  network = ip with all bits beyond prefix zeroed,
 *               prefix  = number of leading 1-bits in the mask
 * 3. Format the subnet as "network/prefix" — this is the deduplication
 *    key.
 * 4. If the key was seen before, skip the address.
 * 5. Otherwise remember the key and append "<ip>[/<prefix>]" to the
 *    result, depending on mask_mode.
 *
 * Loopback (127.0.0.1/8, ::1/128) is handled like any other subnet.
 *
 * ── Platform functions ────────────────────────────────────────────────
 *
 * Linux / POSIX:
 *
 *     int  getifaddrs(struct ifaddrs** ifap);
 *     void freeifaddrs(struct ifaddrs* ifa);
 *
 *   Fills *ifap with a singly-linked list. Each node describes one
 *   (interface, family) pair:
 *
 *       ifa_name     char*            interface name ("eth0", "lo", ...)
 *       ifa_addr     struct sockaddr* address (AF_INET / AF_INET6)
 *       ifa_netmask  struct sockaddr* netmask for that address
 *       ifa_next     struct ifaddrs*  next entry, or NULL
 *
 *   Convert ifa_addr to text with inet_ntop(). Release the whole list
 *   with freeifaddrs().
 *
 * Windows:
 *
 *     ULONG GetAdaptersAddresses(
 *         ULONG  Family,
 *         ULONG  Flags,
 *         PVOID  Reserved,
 *         PIP_ADAPTER_ADDRESSES AdapterAddresses,
 *         PULONG SizePointer);
 *
 *   Two-call pattern:
 *     1. Call with AdapterAddresses = NULL. Returns ERROR_BUFFER_OVERFLOW
 *        and fills *SizePointer with the required buffer size.
 *     2. Allocate the buffer and call again. Success is NO_ERROR.
 *
 *   Returns a singly-linked list of IP_ADAPTER_ADDRESSES (field Next).
 *   Each adapter holds a nested linked list of
 *   IP_ADAPTER_UNICAST_ADDRESS (field Next), with:
 *
 *       Address.lpSockaddr   struct sockaddr*  the IP address
 *       OnLinkPrefixLength   UINT8             subnet prefix length
 *
 *   Link with iphlpapi.lib and Ws2_32.lib.
 *
 * ── Usage ─────────────────────────────────────────────────────────────
 *
 *     int n = 0;
 *     char** ips = get_server_ip_addresses(&n, SERVER_IP_WITH_MASK);
 *     if (ips != NULL) {
 *         print_info("Server IP addresses (%d):\n", n);
 *         for (int i = 0; i < n; i++) {
 *             print_info("  %s\n", ips[i]);
 *         }
 *         free_server_ip_addresses(ips, n);
 *     }
 */

#include "get_server_ip_addresses.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "Ws2_32.lib")
#else
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif


static char* dup_str(const char* src);
static int ip_in_list(const char** list, int n, const char* value);
static char** push_ip(char** ips, int* count, int* capacity, const char* text);
static int prefix_from_mask_v4(uint32_t mask_be);
static int prefix_from_mask_v6(const unsigned char mask[16]);
static int subnet_key_v4(const struct in_addr* ip, int prefix, char* buf, size_t buflen);
static int subnet_key_v6(const struct in6_addr* ip, int prefix, char* buf, size_t buflen);




static char* dup_str(const char* src) {
	size_t len = strlen(src) + 1;
	char* dst = malloc(len);
	if (dst != NULL) {
		memcpy(dst, src, len);
	}

	return dst;
}




static int ip_in_list(const char** list, int n, const char* value) {
	if (list == NULL) {
		return 0;
	}

	for (int i = 0; i < n; i++) {
		if (strcmp(list[i], value) == 0) {
			return 1;
		}
	}

	return 0;
}




static char** push_ip(char** ips, int* count, int* capacity, const char* text) {
	if (*count >= *capacity) {
		int new_cap = (*capacity == 0) ? 8 : (*capacity * 2);
		char** tmp = realloc(ips, (size_t)new_cap * sizeof(char*));
		if (tmp == NULL) {
			return ips;
		}

		ips = tmp;
		*capacity = new_cap;
	}

	char* copy = dup_str(text);
	if (copy == NULL) {
		return ips;
	}

	ips[*count] = copy;
	(*count)++;

	return ips;
}




static int prefix_from_mask_v4(uint32_t mask_be) {
	uint32_t m = ntohl(mask_be);
	int prefix = 0;
	while (m & 0x80000000u) {
		prefix++;
		m <<= 1;
	}

	return prefix;
}




static int prefix_from_mask_v6(const unsigned char mask[16]) {
	int prefix = 0;
	for (int i = 0; i < 16; i++) {
		unsigned char b = mask[i];
		if (b == 0xFF) {
			prefix += 8;
			continue;
		}
		while (b & 0x80) {
			prefix++;
			b <<= 1;
		}
		break;
	}

	return prefix;
}




static int subnet_key_v4(const struct in_addr* ip, int prefix, char* buf, size_t buflen) {
	uint32_t m;
	if (prefix <= 0) {
		m = 0;
	} else if (prefix >= 32) {
		m = 0xFFFFFFFFu;
	} else {
		m = 0xFFFFFFFFu << (32 - prefix);
	}

	struct in_addr net;
	net.s_addr = ip->s_addr & htonl(m);

	char net_str[INET_ADDRSTRLEN];
	if (inet_ntop(AF_INET, &net, net_str, sizeof(net_str)) == NULL) {
		return 0;
	}

	snprintf(buf, buflen, "%s/%d", net_str, prefix);

	return 1;
}




static int subnet_key_v6(const struct in6_addr* ip, int prefix, char* buf, size_t buflen) {
	struct in6_addr net;
	memcpy(&net, ip, sizeof(net));

	int full_bytes = prefix / 8;
	int rem_bits = prefix % 8;
	if (full_bytes < 16) {
		if (rem_bits > 0) {
			net.s6_addr[full_bytes] &= (unsigned char)(0xFF << (8 - rem_bits));
			full_bytes++;
		}
		memset(net.s6_addr + full_bytes, 0, (size_t)(16 - full_bytes));
	}

	char net_str[INET6_ADDRSTRLEN];
	if (inet_ntop(AF_INET6, &net, net_str, sizeof(net_str)) == NULL) {
		return 0;
	}

	snprintf(buf, buflen, "%s/%d", net_str, prefix);

	return 1;
}




#ifdef _WIN32
static char** enumerate(int* out_count, server_ip_mask_mode mask_mode) {
	ULONG size = 0;
	DWORD ret = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX,
	                                  NULL, NULL, &size);
	if (ret != ERROR_BUFFER_OVERFLOW) {
		return NULL;
	}

	PIP_ADAPTER_ADDRESSES addrs = malloc(size);
	if (addrs == NULL) {
		return NULL;
	}

	ret = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX,
	                            NULL, addrs, &size);
	if (ret != NO_ERROR) {
		free(addrs);

		return NULL;
	}

	char** result = NULL;
	int result_count = 0;
	int result_capacity = 0;

	char** seen = NULL;
	int seen_count = 0;
	int seen_capacity = 0;

	for (PIP_ADAPTER_ADDRESSES a = addrs; a != NULL; a = a->Next) {
		for (PIP_ADAPTER_UNICAST_ADDRESS u = a->FirstUnicastAddress;
		     u != NULL; u = u->Next) {
			struct sockaddr* sa = u->Address.lpSockaddr;
			int prefix = (int)u->OnLinkPrefixLength;

			char ip_str[INET6_ADDRSTRLEN];
			char key[INET6_ADDRSTRLEN + 8];

			if (sa->sa_family == AF_INET) {
				struct sockaddr_in* sa4 = (struct sockaddr_in*)sa;
				if (inet_ntop(AF_INET, &sa4->sin_addr, ip_str, sizeof(ip_str)) == NULL) {
					continue;
				}
				if (subnet_key_v4(&sa4->sin_addr, prefix, key, sizeof(key)) == 0) {
					continue;
				}
			} else if (sa->sa_family == AF_INET6) {
				struct sockaddr_in6* sa6 = (struct sockaddr_in6*)sa;
				if (inet_ntop(AF_INET6, &sa6->sin6_addr, ip_str, sizeof(ip_str)) == NULL) {
					continue;
				}
				if (subnet_key_v6(&sa6->sin6_addr, prefix, key, sizeof(key)) == 0) {
					continue;
				}
			} else {
				continue;
			}

			if (ip_in_list((const char**)seen, seen_count, key)) {
				continue;
			}

			char entry[INET6_ADDRSTRLEN + 8];
			if (mask_mode == SERVER_IP_WITH_MASK) {
				snprintf(entry, sizeof(entry), "%s/%d", ip_str, prefix);
			} else {
				snprintf(entry, sizeof(entry), "%s", ip_str);
			}

			seen = push_ip(seen, &seen_count, &seen_capacity, key);
			result = push_ip(result, &result_count, &result_capacity, entry);
		}
	}

	free(addrs);

	if (seen != NULL) {
		for (int i = 0; i < seen_count; i++) {
			free(seen[i]);
		}
		free(seen);
	}

	*out_count = result_count;

	return result;
}
#else
static char** enumerate(int* out_count, server_ip_mask_mode mask_mode) {
	struct ifaddrs* ifaddr = NULL;
	if (getifaddrs(&ifaddr) == -1) {
		return NULL;
	}

	char** result = NULL;
	int result_count = 0;
	int result_capacity = 0;

	char** seen = NULL;
	int seen_count = 0;
	int seen_capacity = 0;

	for (struct ifaddrs* ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL || ifa->ifa_netmask == NULL) {
			continue;
		}

		char ip_str[INET6_ADDRSTRLEN];
		char key[INET6_ADDRSTRLEN + 8];
		int prefix = 0;

		if (ifa->ifa_addr->sa_family == AF_INET) {
			struct sockaddr_in* sa4 = (struct sockaddr_in*)ifa->ifa_addr;
			struct sockaddr_in* m4 = (struct sockaddr_in*)ifa->ifa_netmask;

			if (inet_ntop(AF_INET, &sa4->sin_addr, ip_str, sizeof(ip_str)) == NULL) {
				continue;
			}

			prefix = prefix_from_mask_v4(m4->sin_addr.s_addr);
			if (subnet_key_v4(&sa4->sin_addr, prefix, key, sizeof(key)) == 0) {
				continue;
			}
		} else if (ifa->ifa_addr->sa_family == AF_INET6) {
			struct sockaddr_in6* sa6 = (struct sockaddr_in6*)ifa->ifa_addr;
			struct sockaddr_in6* m6 = (struct sockaddr_in6*)ifa->ifa_netmask;

			if (inet_ntop(AF_INET6, &sa6->sin6_addr, ip_str, sizeof(ip_str)) == NULL) {
				continue;
			}

			prefix = prefix_from_mask_v6(m6->sin6_addr.s6_addr);
			if (subnet_key_v6(&sa6->sin6_addr, prefix, key, sizeof(key)) == 0) {
				continue;
			}
		} else {
			continue;
		}

		if (ip_in_list((const char**)seen, seen_count, key)) {
			continue;
		}

		char entry[INET6_ADDRSTRLEN + 8];
		if (mask_mode == SERVER_IP_WITH_MASK) {
			snprintf(entry, sizeof(entry), "%s/%d", ip_str, prefix);
		} else {
			snprintf(entry, sizeof(entry), "%s", ip_str);
		}

		seen = push_ip(seen, &seen_count, &seen_capacity, key);
		result = push_ip(result, &result_count, &result_capacity, entry);
	}

	freeifaddrs(ifaddr);

	if (seen != NULL) {
		for (int i = 0; i < seen_count; i++) {
			free(seen[i]);
		}
		free(seen);
	}

	*out_count = result_count;

	return result;
}
#endif




char** get_server_ip_addresses(int* count, server_ip_mask_mode mask_mode) {
	if (count == NULL) {
		return NULL;
	}

	*count = 0;

	int found = 0;
	char** ips = enumerate(&found, mask_mode);
	if (ips == NULL) {
		return NULL;
	}

	*count = found;

	return ips;
}




void free_server_ip_addresses(char** ips, int count) {
	if (ips == NULL) {
		return;
	}

	for (int i = 0; i < count; i++) {
		free(ips[i]);
	}

	free(ips);
}
