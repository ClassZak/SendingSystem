#include "PintProcedures.h"

void print_error(const char* format, ...)
{
#ifdef _WIN32
	HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(console_handle, RED_COLOR);
#endif
	va_list args;

#ifndef _WIN32
	printf(RED_COLOR "[������] ");
#else
	printf("[������] ");
#endif

	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	fprintf(stderr, " (���: %d)\n", WSAGetLastError());

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
	printf(GREEN_COLOR "[�����] ");
#else
	printf("[�����] ");
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
	printf(YELLOW_COLOR "[����] ");
#else
	printf("[����] ");
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
