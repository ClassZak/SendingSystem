#pragma once
// ANSI colors

#ifdef _WIN32
#include <windows.h>
#endif


#ifdef _WIN32

#define FOREGROUND_DEFAULT	(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)  // white
#define RED_COLOR			FOREGROUND_RED
#define GREEN_COLOR			FOREGROUND_GREEN
#define YELLOW_COLOR		0xE  // FOREGROUND_RED | FOREGROUND_GREEN
#define RESET_COLOR			FOREGROUND_DEFAULT

#else

#define RED_COLOR			"\x1b[31m"
#define GREEN_COLOR			"\x1b[32m"
#define YELLOW_COLOR		"\x1b[33m"
#define RESET_COLOR			"\033[0m"

#endif

#include <stdarg.h>
#include <stdio.h>


void print_error	(const char* format, ...);
void print_success	(const char* format, ...);
void print_info		(const char* format, ...);


