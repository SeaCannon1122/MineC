#include <stdio.h>

#include <malloc.h>

#include "general/string_allocator.h"


int main(int argc, char* argv[]) {
	
	void* allocator = string_allocator_new();

	char* word0 = string_allocate(allocator, 4096);
	char* word1 = string_allocate(allocator, 2048);
	char* word2 = string_allocate(allocator, 4096);
	char* word3 = string_allocate(allocator, 40);
	char* word4 = string_allocate(allocator, 40);

	string_allocator_destroy(allocator);

	return 0;
}