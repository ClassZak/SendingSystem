#pragma once
#include <stddef.h>


typedef struct connection_t {
	size_t (*read)(void* ctx, void* buf, size_t len);
	size_t (*write)(void* ctx, void* buf, size_t len);
	void (*close)(void* ctx);
	int (*init)(void* ctx);
	void* ctx;
} connection_t;
