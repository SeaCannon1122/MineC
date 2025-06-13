#include "minec_client.h"

//#include <stb_image/stb_image.h>

#include "resources_defines.h"

struct resource_pack_declaration
{
	uint32_t priority;
	uint8_t* path;
};

int compare_resource_pack_declaration_priority(
	const struct resource_pack_declaration* decl0, 
	const struct resource_pack_declaration* decl1
) 
{
	if (decl0->priority < decl1->priority) return -1;
	if (decl0->priority > decl1->priority) return 1;
	return 0;
}

void _resources_load_paths_of_resource_type(struct minec_client* client, void* paths_hashmap, uint8_t* resource_pack_path, uint8_t* sub_layout_file_path)
{
	uint8_t* layout_file_path_components[] = { resource_pack_path, sub_layout_file_path };
	uint8_t* layout_file_path = s_alloc_joined_string(client->dynamic_alloc, layout_file_path_components, 2);

	size_t file_length;
	void* file_data = file_load(layout_file_path, &file_length);


	if (file_data != NULL)
	{
		void* pack_paths_hashmap = hashmap_new(1024, 2);

		hashmap_read_yaml(pack_paths_hashmap, file_data, file_length);
		free(file_data);

		struct hashmap_iterator it;
		hashmap_iterator_start(&it, pack_paths_hashmap);

		struct hashmap_multi_type* val;
		uint8_t* key;

		while (val = hashmap_iterator_next_key_value_pair(&it, &key)) if (val->type == HASHMAP_VALUE_STRING)
		{
			uint8_t* resource_path_components[] = { resource_pack_path, val->data._string };
			uint8_t* resource_path = s_alloc_joined_string(client->dynamic_alloc, resource_path_components, 2);

			hashmap_set_value(paths_hashmap, key, resource_path, HASHMAP_VALUE_STRING);

			s_free(client->dynamic_alloc, resource_path);
		}

		hashmap_delete(pack_paths_hashmap);
	}
	else printf("[RESOURCES] failed to open %s\n", layout_file_path);

	s_free(client->dynamic_alloc, layout_file_path);
}

void resources_create(struct minec_client* client) {

	uint32_t resource_packs_count = hashmap_get_key_count(client->settings.resource_pack_paths_hashmap);
	struct resource_pack_declaration* paths = s_alloc(client->dynamic_alloc, sizeof(struct resource_pack_declaration) * resource_packs_count);

	{
		struct hashmap_iterator it;
		hashmap_iterator_start(&it, client->settings.resource_pack_paths_hashmap);

		struct hashmap_multi_type* val;
		uint8_t* key;

		for (uint32_t i = 0; val = hashmap_iterator_next_key_value_pair(&it, &key); i++)
		{
			paths[i].priority = val->data._int;
			paths[i].path = key;
		}
	}

	qsort(paths, resource_packs_count, sizeof(struct resource_pack_declaration), compare_resource_pack_declaration_priority);

	void* texture_paths_hashmap = hashmap_new(1024, 2);
	void* font_paths_hashmap = hashmap_new(16, 2);
	void* language_paths_hashmap = hashmap_new(16, 2);

	for (uint32_t i = 0; i < resource_packs_count; i++)
	{
		_resources_load_paths_of_resource_type(client, texture_paths_hashmap, paths[i].path, RESOURCE_PACK_TEXTURES_FILE_PATH);
		_resources_load_paths_of_resource_type(client, font_paths_hashmap, paths[i].path, RESOURCE_PACK_FONTS_FILE_PATH);
		_resources_load_paths_of_resource_type(client, language_paths_hashmap, paths[i].path, RESOURCE_PACK_LANGUAGES_FILE_PATH);
	}

	//textures
	{
		client->resource_index.texture_count = 0;
		uint32_t texture_declaration_count = hashmap_get_key_count(texture_paths_hashmap);

		client->resource_index.textures = s_alloc(client->static_alloc, sizeof(struct resources_texture) * texture_declaration_count + 8);

		client->resource_index.texture_token_id_hashmap = hashmap_new((uint32_t)((float)texture_declaration_count * 1.2f) + 1, 2);

		struct hashmap_iterator it;
		hashmap_iterator_start(&it, texture_paths_hashmap);

		struct hashmap_multi_type* val;
		uint8_t* key;

		while (val = hashmap_iterator_next_key_value_pair(&it, &key)) if (val->type == HASHMAP_VALUE_STRING)
		{
			uint32_t comp;
			uint32_t width;
			uint32_t height;
			uint32_t* data;

			if (data = NULL)//stbi_load(val->data._string, &width, &height, &comp, 4))
			{
				client->resource_index.textures[client->resource_index.texture_count].data = data;
				client->resource_index.textures[client->resource_index.texture_count].width = width;
				client->resource_index.textures[client->resource_index.texture_count].height = height;

				hashmap_set_value(client->resource_index.texture_token_id_hashmap, key, &client->resource_index.texture_count, HASHMAP_VALUE_INT);
				client->resource_index.texture_count++;
			}
			else printf("[RESOURCES] failed to load %s\n", val->data._string);
		}
	}

	//fonts
	{
		client->resource_index.pixelchar_font_count = 0;
		uint32_t font_declaration_count = hashmap_get_key_count(font_paths_hashmap);

		client->resource_index.pixelchar_fonts = s_alloc(client->static_alloc, sizeof(struct resources_pixelchar_font) * font_declaration_count + 8);

		client->resource_index.pixelchar_font_token_id_hashmap = hashmap_new((uint32_t)((float)font_declaration_count * 1.2f) + 1, 2);

		struct hashmap_iterator it;
		hashmap_iterator_start(&it, font_paths_hashmap);

		struct hashmap_multi_type* val;
		uint8_t* key;

		while (val = hashmap_iterator_next_key_value_pair(&it, &key)) if (val->type == HASHMAP_VALUE_STRING)
		{
			size_t size;
			void* data;

			if (data = file_load(val->data._string, &size))
			{
				client->resource_index.pixelchar_fonts[client->resource_index.pixelchar_font_count].font_file_data = data;
				client->resource_index.pixelchar_fonts[client->resource_index.pixelchar_font_count].font_file_data_size = size;

				hashmap_set_value(client->resource_index.pixelchar_font_token_id_hashmap, key, &client->resource_index.pixelchar_font_count, HASHMAP_VALUE_INT);
				client->resource_index.pixelchar_font_count++;
			}
			else printf("[RESOURCES] failed to load %s\n", val->data._string);
		}
	}

	//languages
	{
		client->resource_index.language_hashmap_count = 0;
		uint32_t language_declaration_count = hashmap_get_key_count(language_paths_hashmap);

		client->resource_index.language_hashmaps = s_alloc(client->static_alloc, sizeof(void*) * language_declaration_count + 8);

		client->resource_index.language_hashmap_id_hashmap = hashmap_new((uint32_t)((float)language_declaration_count * 1.2f) + 1, 2);

		struct hashmap_iterator it;
		hashmap_iterator_start(&it, language_paths_hashmap);

		struct hashmap_multi_type* val;
		uint8_t* key;

		while (val = hashmap_iterator_next_key_value_pair(&it, &key)) if (val->type == HASHMAP_VALUE_STRING)
		{
			size_t size;
			void* data;

			if (data = file_load(val->data._string, &size))
			{
				client->resource_index.language_hashmaps[client->resource_index.language_hashmap_count] = hashmap_new(1024, 2);

				hashmap_read_yaml(client->resource_index.language_hashmaps[client->resource_index.language_hashmap_count], data, size);

				hashmap_set_value(client->resource_index.language_hashmap_id_hashmap, key, &client->resource_index.language_hashmap_count, HASHMAP_VALUE_INT);
				client->resource_index.language_hashmap_count++;

				free(data);
			}
			else printf("[RESOURCES] failed to load %s\n", val->data._string);
		}
	}


	hashmap_delete(texture_paths_hashmap);
	hashmap_delete(font_paths_hashmap);
	hashmap_delete(language_paths_hashmap);

	s_free(client->dynamic_alloc, paths);
}


void resources_destroy(struct minec_client* client) {

	for (uint32_t i = 0; i < client->resource_index.texture_count; i++) free(client->resource_index.textures[i].data);
	s_free(client->static_alloc, client->resource_index.textures);
	hashmap_delete(client->resource_index.texture_token_id_hashmap);

	for (uint32_t i = 0; i < client->resource_index.pixelchar_font_count; i++) free(client->resource_index.pixelchar_fonts[i].font_file_data);
	s_free(client->static_alloc, client->resource_index.pixelchar_fonts);
	hashmap_delete(client->resource_index.pixelchar_font_token_id_hashmap);

	for (uint32_t i = 0; i < client->resource_index.language_hashmap_count; i++) hashmap_delete(client->resource_index.language_hashmaps[i]);
	s_free(client->static_alloc, client->resource_index.language_hashmaps);
	hashmap_delete(client->resource_index.language_hashmap_id_hashmap);
}
