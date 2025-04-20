#include "minec_client.h"

#include "settings_defines.h"

void settings_create(struct minec_client* client)
{
	client->settings_state.game_settings.video_settings.gui_scale = 2;
	client->settings_state.game_settings.video_settings.fov = 100;

	client->settings_state.resource_pack_paths_hashmap = hashmap_new(20, 2);

	uint8_t* path_components[] = { client->runtime_files_path, DEFAULT_RESOURCE_PACK_PATH };
	uint8_t* path = string_allocate_joined_string(client->string_allocator, path_components, 2);
	uint32_t weight = 0;
	hashmap_set_value(client->settings_state.resource_pack_paths_hashmap, path, &weight, HASHMAP_VALUE_INT);
	string_free(client->string_allocator, path);
}

void settings_destroy(struct minec_client* client)
{
	hashmap_delete(client->settings_state.resource_pack_paths_hashmap);
}

void settings_load(struct minec_client* client)
{

	//game settings
	{
		size_t file_length;
		uint8_t* path_components[] = { client->runtime_files_path, GAME_SETTINGS_FILE_PATH };
		uint8_t* path = string_allocate_joined_string(client->string_allocator, path_components, 2);

		void* file_data = minec_client_load_file(path, &file_length);
		

		if (file_data != NULL)
		{

			void* hashmap = hashmap_new(20, 2);

			hashmap_read_yaml(hashmap, file_data, file_length);
			free(file_data);

			struct hashmap_multi_type* value;

			if (value = hashmap_get_value(hashmap, "gui_scale")) if (value->type == HASHMAP_VALUE_INT)
				client->settings_state.game_settings.video_settings.gui_scale = value->data._int;

			if (value = hashmap_get_value(hashmap, "fov")) if (value->type == HASHMAP_VALUE_INT)
				client->settings_state.game_settings.video_settings.fov = value->data._int;


			hashmap_delete(hashmap);
		}
		else printf("[SETTINGS] failed to open %s\n", path);

		string_free(client->string_allocator, path);
	}

	//resource packs
	{
		size_t file_length;
		uint8_t* path_components[] = { client->runtime_files_path, RESOURCE_PACKS_FILE_PATH };
		uint8_t* path = string_allocate_joined_string(client->string_allocator, path_components, 2);

		void* file_data = minec_client_load_file(path, &file_length);
		

		if (file_data != NULL)
		{
			void* hashmap = hashmap_new(20, 2);

			hashmap_read_yaml(hashmap, file_data, file_length);
			free(file_data);

			struct hashmap_iterator it;
			hashmap_iterator_start(&it, hashmap);

			struct hashmap_multi_type* value;
			uint8_t* key;

			while (value = hashmap_iterator_next_key_value_pair(&it, &key)) if (value->type == HASHMAP_VALUE_INT)
			{
				uint32_t int_value = (value->data._int == 0 ? 1 : value->data._int);
				hashmap_set_value(client->settings_state.resource_pack_paths_hashmap, key, &int_value, HASHMAP_VALUE_INT);
			}

			hashmap_delete(hashmap);
		}
		else printf("[SETTINGS] failed to open %s\n", path);

		string_free(client->string_allocator, path);
	}


}

void settings_save(struct minec_client* client)
{

}