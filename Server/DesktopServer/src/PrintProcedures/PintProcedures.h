#pragma once
// ANSI коды цветов
#ifndef RED_COLOR
#define RED_COLOR		"\x1b[31m"
#endif // !RED_COLOR

#ifndef GREEN_COLOR
#define GREEN_COLOR		"\x1b[32m"
#endif // !GREEN_COLOR

#ifndef YELLOW_COLOR
#define YELLOW_COLOR	"\x1b[33m"
#endif // !YELLOW_COLOR

#ifndef RESET_COLOR
#define RESET_COLOR		"\x1b[0m"
#endif // !RESET_COLOR


#include <stdarg.h>
#include <stdio.h>

void print_error	(const char* format, ...);
void print_success	(const char* format, ...);
void print_info		(const char* format, ...);
