#pragma once

#include <stdint.h>
#include <stddef.h>

	int is_hardware_rng_available();

	int generate_random_bytes(unsigned char* buffer, size_t length);

	int generate_random_int(int* result);

	int generate_random_int_range(int* result, int min, int max);
