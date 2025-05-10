#include "string_allocator.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#define _STRING_ALLOCATOR_ARENAS_PER_EXTENSION 32

struct string_allocator_arena
{
	bool allocated;

	void* memory_pointer;
	size_t size;

	uint32_t allocations_count;
	size_t free_index;
};

struct string_allocator
{
	struct string_allocator_arena* arenas;
	uint32_t arena_count;

	size_t min_arena_size;
};

void* s_allocator_new(size_t min_arena_size)
{
	struct string_allocator* string_allocator = (struct string_allocator*)malloc(sizeof(struct string_allocator));

	if (string_allocator != NULL)
	{
		string_allocator->arena_count = 0;
		string_allocator->min_arena_size = min_arena_size;
	}

	return string_allocator;
}

void* s_alloc(void* allocator, size_t size)
{
	size = (size + 7) / 8 * 8;

	struct string_allocator* string_allocator = (struct string_allocator*)allocator;

	for (uint32_t i = 0; i < string_allocator->arena_count; i++)
	{
		if (string_allocator->arenas[i].allocated == false)
		{
			size_t new_arena_size = ((string_allocator->min_arena_size > size ? string_allocator->min_arena_size : size) + 7) / 8 * 8;

			string_allocator->arenas[i].memory_pointer = (void*)malloc(new_arena_size);
			if (string_allocator->arenas[i].memory_pointer == NULL) return NULL;

			string_allocator->arenas[i].size = new_arena_size;
			string_allocator->arenas[i].allocated = true;
		}

		if (string_allocator->arenas[i].size - string_allocator->arenas[i].free_index >= size)
		{
			void* memory_pointer = (void*)((size_t)string_allocator->arenas[i].memory_pointer + string_allocator->arenas[i].free_index);

			string_allocator->arenas[i].allocations_count++;
			string_allocator->arenas[i].free_index += size;

			return memory_pointer;
		}
	}

	uint32_t arena_count = string_allocator->arena_count;
	struct string_allocator_arena* new_arenas;

	if (arena_count == 0)
		new_arenas = (struct string_allocator_arena*)malloc(sizeof(struct string_allocator_arena) * _STRING_ALLOCATOR_ARENAS_PER_EXTENSION);
	else
		new_arenas = (struct string_allocator_arena*)realloc(string_allocator->arenas, sizeof(struct string_allocator_arena) * (arena_count + 1) * _STRING_ALLOCATOR_ARENAS_PER_EXTENSION);

	if (new_arenas == NULL) return NULL;

	string_allocator->arenas = new_arenas;
	string_allocator->arena_count += _STRING_ALLOCATOR_ARENAS_PER_EXTENSION;

	for (uint32_t i = 0; i < _STRING_ALLOCATOR_ARENAS_PER_EXTENSION; i++)
	{
		new_arenas[arena_count + i].allocated = false;
		new_arenas[arena_count + i].allocations_count = 0;
		new_arenas[arena_count + i].free_index = 0;
	}

	return s_alloc(allocator, size);
}

uint8_t* s_alloc_string(void* allocator, uint8_t* string, ...)
{
	va_list args;
	va_start(args, string);
	int str_len = vsnprintf(NULL, 0, string, args);
	va_end(args);

	if (str_len < 0) return NULL;
	
	uint8_t* str_ptr = s_alloc(allocator, str_len + 1);
	if (str_ptr == NULL) return NULL;

	va_start(args, string);
	int written = vsnprintf(str_ptr, str_len + 1, string, args);
	va_end(args);

	if (written < 0)
	{
		s_free(allocator, str_ptr);
		return NULL;
	}

	return str_ptr;
}

uint8_t* s_alloc_joined_string(void* allocator, uint8_t** strings, uint32_t string_count)
{
	size_t joined_string_length = 0;
	for (uint32_t i = 0; i < string_count; i++) joined_string_length += strlen(strings[i]);

	uint8_t* joined_string = s_alloc(allocator, joined_string_length + 1);
	if (joined_string == NULL) return NULL;

	size_t offset = 0;
	for (uint32_t i = 0; i < string_count; i++)
	{
		size_t string_length = strlen(strings[i]);
		memcpy(&joined_string[offset], strings[i], string_length);
		offset += string_length;
	}

	joined_string[joined_string_length] = 0;

	return joined_string;
}

void s_free(void* allocator, void* memory_handle)
{
	struct string_allocator* string_allocator = (struct string_allocator*)allocator;

	for (uint32_t i = 0; i < string_allocator->arena_count; i++)
	{
		size_t begin = (size_t)string_allocator->arenas[i].memory_pointer;
		size_t end = (size_t)string_allocator->arenas[i].memory_pointer + string_allocator->arenas[i].size;
		
		if ((size_t)memory_handle >= begin && (size_t)memory_handle < end)
		{
			string_allocator->arenas[i].allocations_count--;

			if (string_allocator->arenas[i].allocations_count == 0)
			{
				string_allocator->arenas[i].free_index = 0;
				string_allocator->arenas[i].allocated = false;
				free(string_allocator->arenas[i].memory_pointer);
			}
			return;
		}	
	}
}

void s_allocator_delete(void* allocator)
{
	struct string_allocator* string_allocator = (struct string_allocator*)allocator;

	for (uint32_t i = 0; i < string_allocator->arena_count; i++) if (string_allocator->arenas[i].allocated) free(string_allocator->arenas[i].memory_pointer);

	if (string_allocator->arena_count > 0) free(string_allocator->arenas);
	free(string_allocator);
}