#include "FileService.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <regex.h>


void sanitize_filename(char* dest, size_t dest_size, const char* src)
{
	if (!src)
	{
		strncpy(dest, "unnamed.dat", dest_size);
		return;
	}
	
	regex_t invalid_regex;
	if (regcomp(&invalid_regex, "[^[:alnum:]._-]", REG_EXTENDED))
	{
		strncpy(dest, "unnamed.dat", dest_size);
		return;
	}

	char temp[MAX_FILENAME + 1];
	size_t temp_len = 0;
	const char* p = src;
	regmatch_t match;
	while (*p && temp_len < MAX_FILENAME)
	{
		if (regexec(&invalid_regex, p, 1, &match, 0) == 0)
		{
			// Copy everything before illegal symbol
			size_t len = match.rm_so;
			if (temp_len + len >= MAX_FILENAME) len = MAX_FILENAME - temp_len - 1;
				strncpy(temp + temp_len, p, len);
			temp_len += len;
			if (temp_len < MAX_FILENAME) temp[temp_len++] = '_';
				p += match.rm_eo;
		}
		else
		{
			size_t remaining = strlen(p);
			if (temp_len + remaining >= MAX_FILENAME)
				remaining = MAX_FILENAME - temp_len - 1;
			strncpy(temp + temp_len, p, remaining);
			temp_len += remaining;
			break;
		}
	}
	temp[temp_len] = '\0';
	regfree(&invalid_regex);

	char* dotdot;
	while ((dotdot = strstr(temp, "..")) != NULL)
	{
		dotdot[0] = '_';
		dotdot[1] = '_';
		memmove(dotdot + 1, dotdot + 2, strlen(dotdot + 2) + 1);
	}
	char *final = temp;
	if (final[0] == '.')
		++final;

	if (strlen(final) == 0)
		strncpy(dest, "unnamed.dat", dest_size);
	else
		strncpy(dest, final, dest_size);

	dest[dest_size - 1] = '\0';
}

