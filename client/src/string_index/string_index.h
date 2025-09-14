#pragma once

#ifndef MINEC_CLIENT_STRING_INDEX_STRING_INDEX_H
#define MINEC_CLIENT_STRING_INDEX_STRING_INDEX_H

#include <stdint.h>

#define STRING_INDEX_MAX_LANGUAGE_COUNT 64

struct string_index_language_info
{
	uint32_t* name;
	uint32_t* country;
};

struct string_index
{
	void* language_hashmaps[STRING_INDEX_MAX_LANGUAGE_COUNT];
	uint32_t current_index;
};

uint32_t string_index_create(struct minec_client* client, struct string_index_language_info** language_infos, uint32_t* language_info_count);
void string_index_destroy(struct minec_client* client);

void string_index_set_language_index(struct minec_client* client, uint32_t index);

uint32_t* string_index_query(struct minec_client* client, uint8_t* string);

#endif
