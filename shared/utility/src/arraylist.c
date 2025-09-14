#include <arraylist.h>

#include <stdlib.h>
#include <string_allocator.h>
#include <string.h>

struct _arraylist_string
{
	uint8_t** elements;
	uint32_t element_count;
	uint32_t length;
	uint32_t extension_element_count;

	void* string_allocator;
};

void* arraylist_string_new(uint32_t extension_element_count)
{
	if (extension_element_count < 8) extension_element_count = 8;

	struct _arraylist_string* arraylist = malloc(sizeof(struct _arraylist_string));
	if (arraylist == NULL) return NULL;

	if ((arraylist->string_allocator = s_allocator_new(512)) == NULL)
	{
		free(arraylist);
		return NULL;
	}

	if ((arraylist->elements = malloc(extension_element_count * sizeof(uint8_t*))) == NULL)
	{
		s_allocator_delete(arraylist->string_allocator);
		free(arraylist);
		return NULL;
	}

	arraylist->element_count = extension_element_count;
	arraylist->length = 0;
	arraylist->extension_element_count = extension_element_count;
	return arraylist;
}

void arraylist_string_delete(void* arraylist_string)
{
	struct _arraylist_string* arraylist = arraylist_string;

	for (uint32_t i = 0; i < arraylist->length; i++)
		s_free(arraylist->string_allocator, arraylist->elements[i]);

	free(arraylist->elements);
	s_allocator_delete(arraylist->string_allocator);
	free(arraylist);
}

void* arraylist_string_new_copy(void* arraylist_string)
{
	struct _arraylist_string* arraylist = arraylist_string;
	struct _arraylist_string* arraylist_copy = malloc(sizeof(struct _arraylist_string));

	arraylist_copy->string_allocator = s_allocator_new(512);
	arraylist_copy->elements = malloc(arraylist->element_count * sizeof(uint8_t*));
	arraylist_copy->element_count = arraylist->element_count;
	arraylist_copy->length = arraylist->length;
	arraylist_copy->extension_element_count = arraylist->extension_element_count;

	for (uint32_t i = 0; i < arraylist->length; i++) arraylist_copy->elements[i] = s_alloc_string(arraylist_copy->string_allocator, arraylist->elements[i]);

	return arraylist_copy; 
}

uint32_t arraylist_string_get_length(void* arraylist_string)
{
	struct _arraylist_string* arraylist = arraylist_string;
	return arraylist->length;
}

const uint8_t* arraylist_string_get_element(void* arraylist_string, uint32_t index)
{
	struct _arraylist_string* arraylist = arraylist_string;
	if (index >= arraylist->length) return NULL;
	return (const uint8_t*)arraylist->elements[index];
}

void arraylist_string_set_element(void* arraylist_string, uint32_t index, uint8_t* value)
{
	struct _arraylist_string* arraylist = arraylist_string;
	if (index >= arraylist->length) return;

	s_free(arraylist->string_allocator, arraylist->elements[index]);
	arraylist->elements[index] = s_alloc_string(arraylist->string_allocator, value);
}

void arraylist_string_insert_element(void* arraylist_string, uint32_t index, uint8_t* value)
{
	struct _arraylist_string* arraylist = arraylist_string;

	if (index > arraylist->length) return;

	if (arraylist->length == arraylist->element_count)
	{
		arraylist->element_count += arraylist->extension_element_count;
		arraylist->elements = realloc(arraylist->elements, arraylist->element_count * sizeof(uint8_t*));
	}

	else if (index < arraylist->length)
		memmove(&arraylist->elements[index + 1], &arraylist->elements[index], (arraylist->length - index) * sizeof(uint8_t*));

	arraylist->elements[index] = s_alloc_string(arraylist->string_allocator, value);
	arraylist->length++;
}

void arraylist_string_move_elements(void* arraylist_string, uint32_t dst_index, uint32_t src_index, uint32_t element_count)
{
	struct _arraylist_string* arraylist = arraylist_string;
	if (dst_index + element_count > arraylist->length || src_index + element_count > arraylist->length || src_index == dst_index || element_count == 0) return;

	uint8_t** movees = malloc(element_count * sizeof(uint8_t*));
	memcpy(movees, &arraylist->elements[src_index], element_count * sizeof(uint8_t*));

	if (dst_index > src_index)
		memmove(&arraylist->elements[src_index]
			, &arraylist->elements[src_index + element_count], (dst_index - src_index) * sizeof(uint8_t*));
	else
		memmove(&arraylist->elements[dst_index + element_count], &arraylist->elements[dst_index], (src_index - dst_index) * sizeof(uint8_t*));

	memcpy(&arraylist->elements[dst_index], movees, element_count * sizeof(uint8_t*));
	free(movees);
}

void arraylist_string_swap_elements(void* arraylist_string, uint32_t index0, uint32_t index1, uint32_t element_count)
{
	struct _arraylist_string* arraylist = arraylist_string;
	if (index0 + element_count > arraylist->length || index1 + element_count > arraylist->length) return;

	for (uint32_t i = 0; i < element_count; i++)
	{
		uint8_t* temp = arraylist->elements[index0 + i];
		arraylist->elements[index0 + i] = arraylist->elements[index1 + i];
		arraylist->elements[index1 + i] = temp;
	}
}

void arraylist_string_remove_elements(void* arraylist_string, uint32_t start_index_inc, uint32_t end_index_inc)
{
	struct _arraylist_string* arraylist = arraylist_string;
	if (start_index_inc >= arraylist->length || end_index_inc >= arraylist->length || start_index_inc > end_index_inc) return;

	for (uint32_t i = start_index_inc; i <= end_index_inc; i++) s_free(arraylist->string_allocator, arraylist->elements[i]);

	if (arraylist->length != end_index_inc + 1) memmove(&arraylist->elements[start_index_inc], &arraylist->elements[end_index_inc + 1], (arraylist->length - end_index_inc - 1) * sizeof(uint8_t*));
	arraylist->length -= end_index_inc - start_index_inc + 1;
}

void arraylist_string_read_file_data(void* arraylist_string, const uint8_t* file_data, size_t file_data_size)
{
	if (file_data == NULL || file_data_size == 0) return;
	for (size_t i = 0; i < file_data_size; i++) if (file_data[i] == 0) return;

	uint8_t* data_copy = malloc(file_data_size + 1);
	memcpy(data_copy, file_data, file_data_size);
	data_copy[file_data_size] = '\n';

	size_t string_begin = 0;

	for (size_t i = 0; ; i++)
	{
		if (data_copy[i] == '\n' || data_copy[i] == '\r')
		{
			data_copy[i] = 0;
			if (i - string_begin > 0) arraylist_string_insert_element(arraylist_string, arraylist_string_get_length(arraylist_string), &data_copy[string_begin]);
			string_begin = i + 1;
		}
		if (i == file_data_size) break;
	}

	free(data_copy);
}

uint8_t* arraylist_string_write_file_data(void* arraylist_string, size_t* file_data_size)
{
	struct _arraylist_string* arraylist = arraylist_string;

	if (arraylist->length == 0) return NULL;

	size_t size = 0;

	for (uint32_t i = 0; i < arraylist->length; i++) size += strlen(arraylist->elements[i]) + 1;

	uint8_t* data = malloc(size);
	size_t data_index = 0;

	for (uint32_t i = 0; i < arraylist->length; i++)
	{
		size_t str_len = strlen(arraylist->elements[i]);
		memcpy(&data[data_index], arraylist->elements[i], str_len);
		data[data_index + str_len] = '\n';
		data_index += str_len + 1;
	}

	*file_data_size = size;
	return data;
}