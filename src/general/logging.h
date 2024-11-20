#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

static void log_message(FILE* log_file, const char* format, ...) {
	time_t t = time(NULL);
	struct tm* time_info = localtime(&t);

	char time_buffer[] = {
		 (time_info->tm_mon  +    1) /   10       + '0',
		 (time_info->tm_mon  +    1)         % 10 + '0',
		'/',
		  time_info->tm_mday         /   10       + '0',
		  time_info->tm_mday                 % 10 + '0',
		'/',
		 (time_info->tm_year + 1900) / 1000       + '0',
		((time_info->tm_year + 1900) /  100) % 10 + '0',
		((time_info->tm_year + 1900) /   10) % 10 + '0',
		 (time_info->tm_year + 1900)         % 10 + '0',
		' ',
		  time_info->tm_hour         /   10       + '0',
		  time_info->tm_hour                 % 10 + '0',
		':',
		  time_info->tm_min          /   10       + '0',
		  time_info->tm_min                  % 10 + '0',
		':',
		  time_info->tm_sec          /   10       + '0',
		  time_info->tm_sec                  % 10 + '0',
		' ',
		'\0'
	};

	va_list args;
	va_start(args, format);
	printf(time_buffer);
	vprintf(format, args);
	printf("\n");
	va_end(args);

	va_start(args, format);
	fprintf(log_file, time_buffer);
	vfprintf(log_file, format, args);
	fprintf(log_file, "\n");
	va_end(args);
	fflush(log_file);
}