#pragma once

#ifndef PLATFORM_UTILS_H
#define PLATFORM_UTILS_H

#include <stdint.h>
#include <stdbool.h>

#if defined(_WIN32)

#define DEBUG_BREAK() __debugbreak()
#define RESTRICT __restrict
#define EXPORT __declspec(dllexport)

#elif defined(__linux__)
#include <signal.h>
#define DEBUG_BREAK() raise(SIGTRAP)
#define RESTRICT restrict
#define EXPORT __attribute__((visibility("default")))

#elif defined(__APPLE__)

#define DEBUG_BREAK __builtin_trap()
#define RESTRICT restrict

#endif

void* dynamic_library_load(uint8_t* library_name, bool name_explicit);
void (*dynamic_library_get_function(void* library_handle, uint8_t* function_name)) (void);
void dynamic_library_unload(void* library_handle);

uint32_t directory_exists(uint8_t* path);
uint32_t create_directory(uint8_t* path);
uint32_t get_cwd(uint8_t* buffer, size_t buffer_size);

//general

void set_console_cursor_position(int32_t x, int32_t y);

void sleep_for_ms(uint32_t time_in_milliseconds);

double get_time();

void* create_thread(void (address) (void*), void* args);

void join_thread(void* thread_handle);

uint32_t file_copy(uint8_t* source_path, uint8_t* dest_path);
void* file_load(uint8_t* path, size_t* size);

#endif