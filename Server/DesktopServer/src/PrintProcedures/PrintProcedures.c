#include "PrintProcedures.h"
#ifndef _WIN32
#include <errno.h>
#include <string.h>
#endif

void print_error(const char* format, ...)
{
#ifdef _WIN32
	HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(console_handle, RED_COLOR);
#endif
	va_list args;

#ifndef _WIN32
	fprintf(stderr, RED_COLOR "[ERROR] ");
#else
	printf("[ERROR] ");
#endif

	va_start(args, format);
	vprintf(format, args);
	va_end(args);

#ifdef _WIN32
	fprintf(stderr, "\nCode : %d)\n", GetLastError());
#else
	fprintf(stderr, "\nCode: %d\t%s\n", errno, strerror(errno));
#endif

#ifndef _WIN32
	printf(RESET_COLOR);
#else
	SetConsoleTextAttribute(console_handle, RESET_COLOR);
#endif
}

void print_success(const char* format, ...)
{
#ifdef _WIN32
	HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(console_handle, GREEN_COLOR);
#endif
	va_list args;

#ifndef _WIN32
	printf(GREEN_COLOR "[SUCCESS] ");
#else
	printf("[SUCCESS] ");
#endif
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

#ifndef _WIN32
	printf(RESET_COLOR);
#else
	SetConsoleTextAttribute(console_handle, RESET_COLOR);
#endif
}

void print_info(const char* format, ...)
{
#ifdef _WIN32
	HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(console_handle, YELLOW_COLOR);
#endif
	va_list args;

#ifndef _WIN32
	printf(YELLOW_COLOR "[INFO] ");
#else
	printf("[INFO] ");
#endif
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

#ifndef _WIN32
	printf(RESET_COLOR);
#else
	SetConsoleTextAttribute(console_handle, RESET_COLOR);
#endif
}
