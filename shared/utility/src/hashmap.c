#include "hashmap.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

#include "string_allocator.h"

uint32_t hash_djb2(const uint8_t* str, uint32_t max) {
	uint32_t c, hash = 5381;

	while ((c = *(str++))) hash = ((hash << 5) + hash) + c;

	return hash % max;
}

struct _hashmap_entry
{
	uint8_t* key;
	struct hashmap_multi_type value;
	bool in_use;
};

struct _hashmap_subarray
{
	uint32_t entry_count;
	struct _hashmap_entry* entries;
};

struct _hashmap
{
	void* string_allocator;

	uint32_t sub_array_count;
	uint32_t subarray_extension_mappings_count;

	uint32_t key_count;

	struct _hashmap_subarray sub_arrays[];
};

uint32_t _hashmap_key_exists(struct _hashmap* map, uint8_t* key, uint32_t index, uint32_t* sub_index)
{
	for (uint32_t i = 0; i < map->sub_arrays[index].entry_count; i++)
	{
		if (map->sub_arrays[index].entries[i].in_use)
		{
			if (strcmp(key, map->sub_arrays[index].entries[i].key) == 0)
			{
				*sub_index = i;
				return 1;
			}
		}
	}
	return 0;
}

void _hashmap_key_get_empty_space(struct _hashmap* map, uint8_t* key, uint32_t index, uint32_t* sub_index)
{
	for (uint32_t i = 0; i < map->sub_arrays[index].entry_count; i++)
	{
		if (map->sub_arrays[index].entries[i].in_use == false)
		{
			map->sub_arrays[index].entries[i].in_use = true;
			map->sub_arrays[index].entries[i].key = s_alloc_string(map->string_allocator, key);
			*sub_index = i;
			return;
		}
	}

	uint32_t old_entry_count = map->sub_arrays[index].entry_count;
	map->sub_arrays[index].entry_count += map->subarray_extension_mappings_count;

	if (old_entry_count == 0)
		map->sub_arrays[index].entries = s_alloc(map->string_allocator, sizeof(struct _hashmap_entry) * map->sub_arrays[index].entry_count);
	else
	{
		struct _hashmap_entry* old_entries = map->sub_arrays[index].entries;
		map->sub_arrays[index].entries = s_alloc(map->string_allocator, sizeof(struct _hashmap_entry) * map->sub_arrays[index].entry_count);
		memcpy(map->sub_arrays[index].entries, old_entries, sizeof(struct _hashmap_entry) * old_entry_count);
		s_free(map->string_allocator, old_entries);
	}

	memset((void*)((size_t)map->sub_arrays[index].entries + sizeof(struct _hashmap_entry) * old_entry_count), 0, sizeof(struct _hashmap_entry) * map->subarray_extension_mappings_count);

	map->sub_arrays[index].entries[old_entry_count].in_use = true;
	map->sub_arrays[index].entries[old_entry_count].key = s_alloc_string(map->string_allocator, key);
	*sub_index = old_entry_count;

	return;
}

void* hashmap_new(uint32_t sub_array_count, uint32_t subarray_extension_mappings_count)
{
	if (sub_array_count == 0 || subarray_extension_mappings_count == 0) return NULL;

	void* allocator = s_allocator_new(16384);

	struct _hashmap* map = s_alloc(allocator, sizeof(struct _hashmap) + sizeof(struct _hashmap_subarray) * sub_array_count);

	map->sub_array_count = sub_array_count;
	map->subarray_extension_mappings_count = subarray_extension_mappings_count;
	memset(map->sub_arrays, 0, sizeof(struct _hashmap_subarray) * sub_array_count);
	map->string_allocator = allocator;
	map->key_count = 0;

	return map;
}

void hashmap_delete(void* hashmap)
{
	struct _hashmap* map = hashmap;
	void* allocator = map->string_allocator;

	for (uint32_t i = 0; i < map->sub_array_count; i++)
	{
		for (uint32_t j = 0; j < map->sub_arrays[i].entry_count; j++)
		{
			if (map->sub_arrays[i].entries[j].in_use)
			{
				s_free(allocator, map->sub_arrays[i].entries[j].key);
				if (map->sub_arrays[i].entries[j].value.type == HASHMAP_VALUE_STRING) s_free(allocator, map->sub_arrays[i].entries[j].value.data._string);
			}
		}
		if (map->sub_arrays[i].entry_count > 0) s_free(allocator, map->sub_arrays[i].entries);
	}

	s_free(allocator, map);

	s_allocator_delete(allocator);
}

void hashmap_set_value(void* hashmap, uint8_t* key, void* value, uint32_t value_type)
{
	struct _hashmap* map = hashmap;

	struct hashmap_multi_type resolved_value;
	resolved_value.type = value_type;

	switch (value_type)
	{
	case HASHMAP_VALUE_STRING: {
		resolved_value.data._string = s_alloc_string(map->string_allocator, value);
	} break;
	
	case HASHMAP_VALUE_FLOAT : {
		resolved_value.data._float = *((float*)value);
	} break;

	case HASHMAP_VALUE_INT: {
		resolved_value.data._int = *((int32_t*)value);
	} break;

	default: return;

	}

	uint32_t sub_index, index = hash_djb2(key, map->sub_array_count);
	
	if (_hashmap_key_exists(map, key, index, &sub_index))
	{
		if (map->sub_arrays[index].entries[sub_index].value.type == HASHMAP_VALUE_STRING)
			s_free(map->string_allocator, map->sub_arrays[index].entries[sub_index].value.data._string);
	}
	else
	{
		_hashmap_key_get_empty_space(map, key, index, &sub_index);
		map->key_count++;
	}
		

	map->sub_arrays[index].entries[sub_index].value = resolved_value;
}

struct hashmap_multi_type* hashmap_get_value(void* hashmap, uint8_t* key)
{
	struct _hashmap* map = hashmap;

	uint32_t sub_index, index = hash_djb2(key, map->sub_array_count);

	if (_hashmap_key_exists(map, key, index, &sub_index))
		return &map->sub_arrays[index].entries[sub_index].value;

	return NULL;
}

void hashmap_delete_key(void* hashmap, uint8_t* key)
{
	struct _hashmap* map = hashmap;

	uint32_t sub_index, index = hash_djb2(key, map->sub_array_count);

	if (_hashmap_key_exists(map, key, index, &sub_index))
	{
		if (map->sub_arrays[index].entries[sub_index].value.type == HASHMAP_VALUE_STRING)
			s_free(map->string_allocator, map->sub_arrays[index].entries[sub_index].value.data._string);

		s_free(map->string_allocator, map->sub_arrays[index].entries[sub_index].key);
		map->sub_arrays[index].entries[sub_index].in_use = false;

		map->key_count--;
	}
}

uint32_t hashmap_get_key_count(void* hashmap)
{
	struct _hashmap* map = hashmap;
	return map->key_count;
}

void hashmap_iterator_start(struct hashmap_iterator* iterator, void* hashmap)
{
	iterator->map = hashmap;
	iterator->index = 0;
	iterator->sub_index = 0;
}

struct hashmap_multi_type* hashmap_iterator_next_key_value_pair(struct hashmap_iterator* iterator, uint8_t** key)
{
	struct _hashmap* map = iterator->map;

	for (; iterator->index < map->sub_array_count; iterator->index++)
	{
		for (; iterator->sub_index < map->sub_arrays[iterator->index].entry_count; iterator->sub_index++)
		{
			if (map->sub_arrays[iterator->index].entries[iterator->sub_index].in_use)
			{
				*key = map->sub_arrays[iterator->index].entries[iterator->sub_index].key;
				struct hashmap_multi_type* ret = &map->sub_arrays[iterator->index].entries[iterator->sub_index].value;

				iterator->sub_index++;
				return ret;
			}
		}
		iterator->sub_index = 0;
	}

	return NULL;
}

void hashmap_read_yaml(void* hashmap, uint8_t* yaml_data, size_t yaml_data_size)
{
	struct _hashmap* map = hashmap;

	uint8_t* data = (uint8_t*)malloc(yaml_data_size + 1);
	memcpy(data, yaml_data, yaml_data_size);
	data[yaml_data_size] = '\0';

	uint32_t line_breaks = 0;
	for (uint32_t i = 0; i < yaml_data_size; i++) if (yaml_data[i] == '\n') line_breaks++;

	uint32_t text_i = 0;
	for (uint32_t lines_i = 0; lines_i < line_breaks + 1; lines_i++) {

#define skip_spaces while(data[text_i] == ' ') text_i++
#define continue_on_next_line for(;data[text_i] != '\n' && data[text_i + 1] != '\0'; text_i++) {} text_i++;  continue

		skip_spaces;

		uint32_t key_start = text_i;
		for (; data[text_i] != ' ' && data[text_i] != ':' && data[text_i] != '\n' && data[text_i] != '\0' && data[text_i] != '\r'; text_i++);
		uint32_t key_end = text_i;
		if (key_end - key_start == 0) { continue_on_next_line; }

		skip_spaces;

		if (data[text_i] != ':') { continue_on_next_line; }
		text_i++;
		data[key_end] = '\0';
		skip_spaces;

		uint32_t type = HASHMAP_VALUE_INT;
		uint32_t string_quotes = 0;

		uint32_t value_start = text_i;

		for (; data[text_i] != '\n' && data[text_i] != '\r' && data[text_i] != '\0' && (data[text_i] != ' ' || string_quotes); text_i++) {
			if (data[text_i] == '"') { type = HASHMAP_VALUE_STRING; string_quotes ^= 1; }
			if (type == HASHMAP_VALUE_STRING) continue;
			if (data[text_i] == '.' && type == HASHMAP_VALUE_INT) type = HASHMAP_VALUE_FLOAT;
			else if ((data[text_i] < '0' || data[text_i] > '9') && (data[text_i] != '-' || text_i != value_start)) { type = HASHMAP_VALUE_STRING; }
		}

		if (text_i - value_start == 0) { continue_on_next_line; }

		if (type == HASHMAP_VALUE_STRING) {
			if (data[text_i - 1] == '"' && string_quotes == 0) {
				data[text_i - 1] = '\0';
				hashmap_set_value(map, &data[key_start], &data[value_start + 1], HASHMAP_VALUE_STRING);
			}
			else {
				uint8_t end_val = data[text_i];
				data[text_i] = '\0';
				hashmap_set_value(map, &data[key_start], &data[value_start], HASHMAP_VALUE_STRING);
				data[text_i] = end_val;
			}
		}
		else if (type == HASHMAP_VALUE_INT) {
			int32_t int_val = 0;
			int32_t negative = 1;

			if (data[value_start] == '-') { value_start++; negative = -1; }
			for (uint32_t _i = value_start; _i < text_i; _i++) {
				int_val *= 10;
				int_val += data[_i] - '0';
			}
			int_val *= negative;
			hashmap_set_value(map, &data[key_start], &int_val, HASHMAP_VALUE_INT);
		}
		else {
			float float_val = 0.f;
			float decimal = -1.f;
			float negative = 1.f;

			if (data[value_start] == '-') { value_start++; negative = -1.f; }

			for (uint32_t _i = value_start; _i < text_i; _i++) {
				if (data[_i] == '.') decimal = 10.f;
				else if (decimal < 0) {
					float_val *= 10.f;
					float_val += (float)(data[_i] - '0');
				}
				else {
					float_val += (float)(data[_i] - '0') / decimal;
					decimal *= 10.f;
				}
			}
			float_val *= negative;
			hashmap_set_value(map, &data[key_start], &float_val, HASHMAP_VALUE_FLOAT);
		}

		continue_on_next_line;
	}

	free(data);
}

#define _HAHSMAP_WRITE_DATA_BUFFER_EXTENSION_SIZE 4096

void _hashmap_buffer_write(void** buffer, size_t* buffer_size, size_t* buffer_filled_size, uint8_t* format, ...) {
	va_list args;
	va_start(args, format);

	int required_size = vsnprintf(NULL, 0, format, args);
	va_end(args);

	if (required_size > *buffer_size - *buffer_filled_size) {
		*buffer_size += (required_size > _HAHSMAP_WRITE_DATA_BUFFER_EXTENSION_SIZE ? required_size : _HAHSMAP_WRITE_DATA_BUFFER_EXTENSION_SIZE);
		*buffer = realloc(*buffer, *buffer_size);
	}

	va_start(args, format);
	*buffer_filled_size += vsnprintf(buffer[*buffer_filled_size], *buffer_size - *buffer_filled_size, format, args);
	va_end(args);
}

void* hashmap_write_yaml(void* hashmap, size_t* yaml_data_size)
{
	void* buffer = malloc(_HAHSMAP_WRITE_DATA_BUFFER_EXTENSION_SIZE);
	size_t buffer_size = _HAHSMAP_WRITE_DATA_BUFFER_EXTENSION_SIZE;
	size_t buffer_filled_size = 0;

	struct hashmap_iterator iterator;
	hashmap_iterator_start(&iterator, hashmap);

	uint8_t* key;
	struct hashmap_multi_type* value;

	while (value = hashmap_iterator_next_key_value_pair(&iterator, &key))
	{
		switch (value->type)
		{
		case HASHMAP_VALUE_STRING: {
			_hashmap_buffer_write(&buffer, &buffer_size, &buffer_filled_size, "%s: \"%s\"\n", key, value->data._string);
		} break;

		case HASHMAP_VALUE_FLOAT: {
			_hashmap_buffer_write(&buffer, &buffer_size, &buffer_filled_size, "%s: \"%f\"\n", key, value->data._float);
		} break;

		case HASHMAP_VALUE_INT: {
			_hashmap_buffer_write(&buffer, &buffer_size, &buffer_filled_size, "%s: \"%d\"\n", key, value->data._int);
		} break;

		}
	}

	_hashmap_buffer_write(&buffer, &buffer_size, &buffer_filled_size, " ");

	*yaml_data_size = buffer_filled_size;
	return buffer;
}