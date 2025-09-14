#include <minec_client.h>

#include "string_index/english.c"
#include "string_index/german.c"

#define LANGUAGE_LIST \
	LANGUAGE(english, 0)\
	LANGUAGE(german, 1)\

#define LANGUAGE_COUNT 2

struct string_index_language_info language_infos[LANGUAGE_COUNT] =
{
#define LANGUAGE(name, index) name##_language_info,
	LANGUAGE_LIST
#undef LANGUAGE
};

uint32_t string_index_create(struct minec_client* client, struct string_index_language_info** infos, uint32_t* info_count)
{

	for (uint32_t i = 0; i < LANGUAGE_COUNT; i++) if ((client->string_index.language_hashmaps[i] = hashmap_new(1024, 2)) == NULL)
	{
		for (uint32_t j = 0; j < i; j++) hashmap_delete(client->string_index.language_hashmaps[j]);
		return MINEC_CLIENT_ERROR;
	}

#define LANGUAGE(name, index) string_index_##name##_init(client->string_index.language_hashmaps[index]);
	LANGUAGE_LIST
#undef LANGUAGE

	*info_count = LANGUAGE_COUNT;
	*infos = language_infos;

	return MINEC_CLIENT_SUCCESS;
}

void string_index_destroy(struct minec_client* client)
{
	for (uint32_t i = 0; i < LANGUAGE_COUNT; i++) hashmap_delete(client->string_index.language_hashmaps[i]);
}

void string_index_set_language_index(struct minec_client* client, uint32_t index)
{
	if (index < LANGUAGE_COUNT) client->string_index.current_index = index;
}

uint32_t* string_index_query(struct minec_client* client, uint8_t* string)
{
	struct hashmap_multi_type* value = hashmap_get_value(client->string_index.language_hashmaps[client->string_index.current_index], string);

	if (value) return value->data_string32;
	return U"STRING_NOT_FOUND";
}
