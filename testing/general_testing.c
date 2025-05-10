
#include <string_allocator.h>
#include <stdio.h>

int main()
{
	void* allocator = string_allocator_new(4096);

	uint8_t* string = string_allocate_string(allocator, "String: '%s'\nInteger: %d\ndouble: %llf", "Hello World!", -187, 3.141);

	printf(string);

	string_allocator_delete(allocator);
}