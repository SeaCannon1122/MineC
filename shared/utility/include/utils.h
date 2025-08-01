#pragma once

#ifndef PLATFORM_UTILS_H
#define PLATFORM_UTILS_H

#include <stdint.h>
#include <stdbool.h>

#if defined(_WIN32)

#ifndef NDEBUG
#define DEBUGBREAK() __debugbreak()
#else
#define DEBUGBREAK()
#endif

#define RESTRICT __restrict
#define EXPORT __declspec(dllexport)

#elif defined(__linux__)

#ifndef NDEBUG
#include <signal.h>
#define DEBUGBREAK() raise(SIGTRAP)
#else
#define DEBUGBREAK()
#endif

#define RESTRICT restrict
#define EXPORT __attribute__((visibility("default")))

#endif

//dynamic_library
void* dynamic_library_load(uint8_t* library_name, bool name_explicit);
void (*dynamic_library_get_function(void* library_handle, uint8_t* function_name)) (void);
void dynamic_library_unload(void* library_handle);

//time
void time_sleep(uint32_t time_in_milliseconds);
float time_get();

//thread
void* thread_create(void (address) (void*), void* args);
void thread_join(void* thread_handle);

//file
uint32_t file_copy(uint8_t* src_path, uint8_t* dst_path);
void* file_load(uint8_t* path, size_t* size);

#endif