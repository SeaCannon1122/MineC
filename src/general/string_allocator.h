#pragma once

#ifndef STRING_ALLOCATOR_H
#define STRING_ALLOCATOR_H

#include <stdint.h>

void* string_allocator_new(size_t min_arena_size);

void* string_allocate(void* allocator, size_t size);

uint8_t* string_allocate_string(void* allocator, uint8_t* string);

uint32_t string_free(void* allocator, void* memory_handle);

void string_allocator_destroy(void* allocator);

#endif // !STRING_ALLOCATOR_H
