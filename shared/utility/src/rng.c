#include "rng.h"

#if defined(_WIN32)

#include <windows.h>
#include <bcrypt.h>
#include <immintrin.h>  // For RDRAND intrinsics
#include <stdint.h>

#pragma comment(lib, "bcrypt.lib")

static int has_hardware_rng = -1;

static int check_hardware_rng() {
    int info[4];
    __cpuid(info, 1);

    // Check for RDRAND support (bit 30 of ECX)
    return (info[2] & (1 << 30)) != 0;
}

int is_hardware_rng_available() {
    if (has_hardware_rng == -1) {
        has_hardware_rng = check_hardware_rng();
    }
    return has_hardware_rng;
}

static int rdrand_fill_buffer(unsigned char* buffer, size_t length) {
    size_t i;
    for (i = 0; i < length; i += sizeof(uint32_t)) {
        uint32_t rand;
        if (_rdrand32_step(&rand) != 1) {
            return 0;  // Failed to generate random number
        }
        memcpy(buffer + i, &rand, sizeof(uint32_t));
    }
    return 1;  // Success
}

int generate_random_bytes(unsigned char* buffer, size_t length) {
    if (is_hardware_rng_available()) {
        if (rdrand_fill_buffer(buffer, length)) {
            return 1;  // Success
        }
    }

    // Fallback to Windows Cryptographic API
    if (BCryptGenRandom(NULL, buffer, (ULONG)length, BCRYPT_USE_SYSTEM_PREFERRED_RNG) == 0) {
        return 1;  // Success
    }

    return 0;  // Failure
}

int generate_random_int(int* result) {
    return generate_random_bytes((unsigned char*)result, sizeof(int));
}

int generate_random_int_range(int* result, int min, int max) {
    int random_value;
    if (!generate_random_int(&random_value)) {
        return 0;
    }
    *result = min + (random_value % (max - min + 1));
    return 1;
}

#elif defined(__linux__)

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <cpuid.h>

#define RDRAND_SUCCESS 1

static int has_hardware_rng = -1;

static int check_hardware_rng() {
    unsigned int eax, ebx, ecx, edx;
    __cpuid(1, eax, ebx, ecx, edx);

    // Check for RDRAND support (bit 30 of ECX)
    return (ecx & (1 << 30)) != 0;
}

int is_hardware_rng_available() {
    if (has_hardware_rng == -1) {
        has_hardware_rng = check_hardware_rng();
    }
    return has_hardware_rng;
}

static int rdrand32_step(uint32_t* rand) {
    unsigned char success;
    asm volatile("rdrand %0; setc %1"
        : "=r" (*rand), "=qm" (success));
    return success;
}

static int rdrand_fill_buffer(unsigned char* buffer, size_t length) {
    size_t i;
    for (i = 0; i < length; i += sizeof(uint32_t)) {
        uint32_t rand;
        if (!rdrand32_step(&rand)) {
            return 0;  // Failed to generate random number
        }
        memcpy(buffer + i, &rand, sizeof(uint32_t));
    }
    return 1;  // Success
}

int generate_random_bytes(unsigned char* buffer, size_t length) {
    if (is_hardware_rng_available()) {
        if (rdrand_fill_buffer(buffer, length)) {
            return 1;  // Success
        }
    }

    // Fallback to /dev/urandom
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        return 0;  // Failure
    }
    ssize_t bytes_read = read(fd, buffer, length);
    close(fd);

    return bytes_read == length ? 1 : 0;
}

int generate_random_int(int* result) {
    return generate_random_bytes((unsigned char*)result, sizeof(int));
}

int generate_random_int_range(int* result, int min, int max) {
    int random_value;
    if (!generate_random_int(&random_value)) {
        return 0;
    }
    *result = min + (random_value % (max - min + 1));
    return 1;
}

#endif