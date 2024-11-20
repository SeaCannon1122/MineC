#include "utils.h"

#include <stdio.h>
#include <time.h>
#include <stdarg.h>

int clamp_int(int val, int min, int max) {
	return (val < min ? min : (val > max ? max : val));
}

float clamp_float(float val, float min, float max) {
	return (val < min ? min : (val > max ? max : val));
}

int min_int(int a, int b) {
	return (a < b ? a : b);
}

int max_int(int a, int b) {
	return (a > b ? a : b);
}

int string_to_int(char* src, int length) {
	if (length < 1) return 0;
	else if (src[0] == '-' && length == 1) return 0;

	int integer = 0;
	int sign = 1;
	int i = 0;
	if (src[0] == '-') { sign = -1; i++; }

	for (; i < length && src[i] != '\0'; i++) {
		integer *= 10;

		if (src[i] < '0' || src[i] > '9') return 0;
		integer += clamp_int(src[i] - '0', 0, 9);
	}

	return integer * sign;
}

float string_to_float(char* src, int length) {
	if (length < 1) return 0;
	else if (src[0] == '-' && length == 1) return 0;

	float floating = 0;
	int sign = 1;
	int i = 0;
	if (src[0] == '-') { sign = -1; i++; }

	for (; i < length && src[i] != '.'; i++) {
		floating *= 10;

		if (src[i] < '0' || src[i] > '9') return 0;
		floating += (float)(src[i] - '0');
	}
	i++;

	float decimal = 1;
	for (; i < length; i++) {
		decimal *= 10;

		if (src[i] < '0' || src[i] > '9') return 0;
		floating += (float)(src[i] - '0') / decimal;
	}

	return floating * sign;
}

int string_length(char* str) {
	int length = 1;
	for (; str[length - 1] != '\0'; length++);
	return length;
}

void get_time_in_string(char* buffer, time_t raw_time) {

	struct tm* time_info = localtime(&raw_time);

	buffer[0] = ((time_info->tm_mon + 1) / 10) + '0';
	buffer[1] = ((time_info->tm_mon + 1) % 10) + '0';
	buffer[2] = '/';
	buffer[3] = (time_info->tm_mday / 10) + '0';
	buffer[4] = (time_info->tm_mday % 10) + '0';
	buffer[5] = '/';
	buffer[6] = ((time_info->tm_year + 1900) / 1000) + '0';
	buffer[7] = (((time_info->tm_year + 1900) / 100) % 10) + '0';
	buffer[8] = (((time_info->tm_year + 1900) / 10) % 10) + '0';
	buffer[9] = ((time_info->tm_year + 1900) % 10) + '0';
	buffer[10] = ' ';
	buffer[11] = (time_info->tm_hour / 10) + '0';
	buffer[12] = (time_info->tm_hour % 10) + '0';
	buffer[13] = ':';
	buffer[14] = (time_info->tm_min / 10) + '0';
	buffer[15] = (time_info->tm_min % 10) + '0';
	buffer[16] = ':';
	buffer[17] = (time_info->tm_sec / 10) + '0';
	buffer[18] = (time_info->tm_sec % 10) + '0';
}