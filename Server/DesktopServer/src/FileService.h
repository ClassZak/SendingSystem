#pragma once
#define SAVE_DIR "build/saves"
#define SAVE_DIR_LENGTH 11
#define MAX_FILENAME 255

#include <stddef.h>

void sanitize_filename(char* dest, size_t dest_size, const char* src);

