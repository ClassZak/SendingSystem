#include "PintProcedures.h"

void print_error(const char* format, ...)
{
	va_list args;

	printf(RED_COLOR "[Œÿ»¡ ¿] ");

	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	fprintf(stderr, " (ÍÓ‰: %d)\n" RESET_COLOR, WSAGetLastError());
}

void print_success(const char* format, ...)
{
	va_list args;

	printf(GREEN_COLOR "[”—œ≈’] ");

	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	printf(RESET_COLOR);
}

void print_info(const char* format, ...)
{
	va_list args;

	printf(YELLOW_COLOR "[»Õ‘Œ] ");

	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	printf(RESET_COLOR);
}
