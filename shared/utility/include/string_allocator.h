#pragma once

#ifndef STRING_ALLOCATOR_H
#define STRING_ALLOCATOR_H

#include <stdint.h>
#include <stddef.h>

void* s_allocator_new(size_t min_arena_size);
void s_allocator_delete(void* allocator);

void* s_alloc(void* allocator, size_t size);
uint8_t* s_alloc_string(void* allocator, uint8_t* string, ...);
void s_free(void* allocator, void* memory_handle);

#endif
