#pragma once

#include <stdio.h>
#include <stdarg.h>

#include "general/utils.h"

static void log_message(FILE* log_file, const char* format, ...) {
	char time_buffer[21];
	get_time_in_string(time_buffer);
	time_buffer[19] = ' ';
	time_buffer[20] = '\0';

	va_list args;
	va_start(args, format);
	printf(time_buffer);
	vprintf(format, args);
	va_end(args);

	va_start(args, format);
	fprintf(log_file, time_buffer);
	vfprintf(log_file, format, args);
	va_end(args);
	fflush(log_file);
}