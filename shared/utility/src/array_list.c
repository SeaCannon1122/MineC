#include <array_list.h>

#include <stdlib.h>
#include <string_allocator.h>
#include <string.h>

struct _array_list_string
{
	uint8_t** elements;
	uint32_t element_count;
	uint32_t length;
	uint32_t extension_element_count;

	void* string_allocator;
};

void* array_list_string_new(uint32_t extension_element_count)
{
	if (extension_element_count < 8) extension_element_count = 8;

	struct _array_list_string* array_list = malloc(sizeof(struct _array_list_string));
	if (array_list == NULL) return NULL;

	if ((array_list->string_allocator = s_allocator_new(4096)) == NULL)
	{
		free(array_list);
		return NULL;
	}

	if ((array_list->elements = malloc(extension_element_count * sizeof(uint8_t*))) == NULL)
	{
		s_allocator_delete(array_list->string_allocator);
		free(array_list);
		return NULL;
	}

	array_list->element_count = extension_element_count;
	array_list->length = 0;
	array_list->extension_element_count = extension_element_count;
	return array_list;
}

void array_list_string_delete(void* array_list_string)
{
	struct _array_list_string* array_list = array_list_string;

	for (uint32_t i = 0; i < array_list->length; i++)
		s_free(array_list->string_allocator, array_list->elements[i]);

	free(array_list->elements);
	s_allocator_delete(array_list->string_allocator);
	free(array_list);
}

uint32_t array_list_string_get_length(void* array_list_string)
{
	struct _array_list_string* array_list = array_list_string;
	return array_list->length;
}

const uint8_t* array_list_string_get_element(void* array_list_string, uint32_t index)
{
	struct _array_list_string* array_list = array_list_string;
	if (index >= array_list->length) return NULL;
	return (const uint8_t*)array_list->elements[index];
}

void array_list_string_set_element(void* array_list_string, uint32_t index, uint8_t* value)
{
	struct _array_list_string* array_list = array_list_string;
	if (index >= array_list->length) return;

	s_free(array_list->string_allocator, array_list->elements[index]);
	array_list->elements[index] = s_alloc_string(array_list->string_allocator, value);
}

void array_list_string_insert_element(void* array_list_string, uint32_t index, uint8_t* value)
{
	struct _array_list_string* array_list = array_list_string;

	if (index > array_list->length) return;

	if (array_list->length == array_list->element_count)
	{
		array_list->element_count += array_list->extension_element_count;
		array_list->elements = realloc(array_list->elements, array_list->element_count * sizeof(uint8_t*));
	}

	else if (index < array_list->length)
		memmove(&array_list->elements[index + 1], &array_list->elements[index], (array_list->length - index) * sizeof(uint8_t*));

	array_list->elements[index] = s_alloc_string(array_list->string_allocator, value);
	array_list->length++;
}

void array_list_string_move_elements(void* array_list_string, uint32_t dst_index, uint32_t src_index, uint32_t element_count)
{
	struct _array_list_string* array_list = array_list_string;
	if (dst_index + element_count > array_list->length || src_index + element_count > array_list->length || src_index == dst_index || element_count == 0) return;

	uint8_t** movees = malloc(element_count * sizeof(uint8_t*));
	memcpy(movees, &array_list->elements[src_index], element_count * sizeof(uint8_t*));

	if (dst_index > src_index)
		memmove(&array_list->elements[src_index]
			, &array_list->elements[src_index + element_count], (dst_index - src_index) * sizeof(uint8_t*));
	else
		memmove(&array_list->elements[dst_index + element_count], &array_list->elements[dst_index], (src_index - dst_index) * sizeof(uint8_t*));

	memcpy(&array_list->elements[dst_index], movees, element_count * sizeof(uint8_t*));
	free(movees);
}

void array_list_string_swap_elements(void* array_list_string, uint32_t index0, uint32_t index1, uint32_t element_count)
{
	struct _array_list_string* array_list = array_list_string;
	if (index0 + element_count > array_list->length || index1 + element_count > array_list->length) return;

	uint8_t** temp_elements = malloc(element_count * sizeof(uint8_t*));
	memcpy(temp_elements, &array_list->elements[index0], element_count * sizeof(uint8_t*));
	memcpy(&array_list->elements[index0], &array_list->elements[index1], element_count * sizeof(uint8_t*));
	memcpy(&array_list->elements[index1], temp_elements, element_count * sizeof(uint8_t*));
	free(temp_elements);
}

void array_list_string_remove_elements(void* array_list_string, uint32_t start_index_inc, uint32_t end_index_inc)
{
	struct _array_list_string* array_list = array_list_string;
	if (start_index_inc >= array_list->length || end_index_inc >= array_list->length || start_index_inc > end_index_inc) return;

	for (uint32_t i = start_index_inc; i <= end_index_inc; i++) s_free(array_list->string_allocator, array_list->elements[i]);

	if (array_list->length > end_index_inc + 1) memmove(&array_list->elements[start_index_inc], &array_list->elements[end_index_inc + 1], (array_list->length - end_index_inc - 1) * sizeof(uint8_t*));
}

void array_list_string_read_file_data(void* array_list_string, const uint8_t* file_data, size_t file_data_size)
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
			if (i - string_begin > 0) array_list_string_insert_element(array_list_string, array_list_string_get_length(array_list_string), data_copy[string_begin]);
			string_begin = i + 1;
		}
		if (i == file_data_size) break;
	}

	free(data_copy);
}

uint8_t* array_list_string_write_file_data(void* array_list_string, size_t* file_data_size)
{
	struct _array_list_string* array_list = array_list_string;

	if (array_list->length == 0) return;

	size_t size = 0;

	for (uint32_t i = 0; i < array_list->length; i++) size += strlen(array_list->elements[i]) + 1;

	uint8_t* data = malloc(size);
	size_t data_index = 0;

	for (uint32_t i = 0; i < array_list->length; i++)
	{
		size_t str_len = strlen(array_list->elements[i]);
		memcpy(&data[data_index], array_list->elements[i], str_len);
		data[data_index + str_len] = '\n';
		data_index += str_len + 1;
	}

	*file_data_size = size;
	return data;
}