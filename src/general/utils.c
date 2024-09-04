#include "utils.h"

int clamp_int(int val, int min, int max) {
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

	for (; i < length; i++) {
		integer *= 10;

		if (src[i] < '0' || src[i] > '9') return 0;
		integer += src[i] - '0';
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