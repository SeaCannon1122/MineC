#include "string_allocator.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct string_allocator_arena
{
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

void* string_allocator_new(size_t min_arena_size)
{
	struct string_allocator* string_allocator = (struct string_allocator*)malloc(sizeof(struct string_allocator));

	if (string_allocator != NULL)
	{
		string_allocator->arena_count = 0;
		string_allocator->min_arena_size = min_arena_size;
	}

	return string_allocator;
}

void* string_allocate(void* allocator, size_t size)
{
	struct string_allocator* string_allocator = (struct string_allocator*)allocator;

	for (uint32_t i = 0; i < string_allocator->arena_count; i++)
	{
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
	size_t new_arena_size = (string_allocator->min_arena_size > size ? string_allocator->min_arena_size : size);

	if (arena_count == 0)
	{
		new_arenas = (struct string_allocator_arena*)malloc(sizeof(struct string_allocator_arena));
	}
	else
	{
		new_arenas = (struct string_allocator_arena*)realloc(string_allocator->arenas, sizeof(struct string_allocator_arena) * (arena_count + 1));
	}
	if (new_arenas == NULL) return NULL;

	new_arenas[arena_count + 1].memory_pointer = (void*)malloc(new_arena_size);
	if (new_arenas[arena_count + 1].memory_pointer == NULL) return NULL;

	new_arenas[arena_count + 1].size = new_arena_size;
	new_arenas[arena_count + 1].allocations_count = 0;
	new_arenas[arena_count + 1].free_index = 0;

	string_allocator->arenas = new_arenas;
	string_allocator->arena_count++;

	return string_allocate(allocator, size);
}

uint32_t string_free(void* allocator, void* memory_handle)
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
			}

			return 0;
		}	
	}

	return 1;
}

void string_allocator_destroy(void* allocator)
{
	struct string_allocator* string_allocator = (struct string_allocator*)allocator;

	for (uint32_t i = 0; i < string_allocator->arena_count; i++)
	{
		free(string_allocator->arenas[i].memory_pointer);
	}

	free(string_allocator->arenas);
	free(string_allocator);
}