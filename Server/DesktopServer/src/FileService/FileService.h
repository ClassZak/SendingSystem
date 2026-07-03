#pragma once
#define SAVE_DIR "build/saves"
#define MAX_FILENAME 255
#define SAVE_DIR_LENGTH 11


#include <stddef.h>
#ifdef _WIN32
#include <direct.h>   // _mkdir
#else
#include <sys/stat.h> // mkdir
#endif

void sanitize_filename(char* dest, size_t dest_size, const char* src);

