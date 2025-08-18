#include "minec_client.h"

#include "settings_defines.h"

uint32_t settings_create(struct minec_client* client)
{
	uint8_t* path_components[2] = { client->data_files_path };

	path_components[1] = VIDEO_SETTINGS_FILE_PATH;
	SETTINGS.video_settings_file_path = malloc_joined_string(path_components, 2);
	path_components[1] = ACTIVE_RESOURCEPACKS_FILE_PATH;
	SETTINGS.active_resourcepack_file_path = malloc_joined_string(path_components, 2);
	path_components[1] = INACTIVE_RESOURCEPACKS_FILE_PATH;
	SETTINGS.inactive_resourcepack_file_path = malloc_joined_string(path_components, 2);

	SETTINGS.video.gui_scale = 0;
	SETTINGS.video.renderer.fov = 120;
	SETTINGS.video.renderer.render_distance = 8;
	SETTINGS.video.renderer.backend_index = 0;
	SETTINGS.video.renderer.backend_device_index = 0;
	SETTINGS.video.renderer.fps = 0;
	SETTINGS.video.renderer.vsync = true;
	SETTINGS.video.renderer.max_mipmap_level_count = 5;
	SETTINGS.video.renderer.order_create_new_destroy_old = true;

	SETTINGS.active_resourcepack_paths_arraylist = arraylist_string_new(8);
	SETTINGS.inactive_resourcepack_paths_arraylist = arraylist_string_new(8);

	return MINEC_CLIENT_SUCCESS;
}

void settings_destroy(struct minec_client* client)
{
	arraylist_string_delete(SETTINGS.active_resourcepack_paths_arraylist);
	arraylist_string_delete(SETTINGS.inactive_resourcepack_paths_arraylist);

	free(SETTINGS.active_resourcepack_file_path);
	free(SETTINGS.inactive_resourcepack_file_path);
	free(SETTINGS.video_settings_file_path);
}

void settings_load(struct minec_client* client)
{
	//video settings
	{
		size_t file_length;
		void* yaml_file_data = file_load(SETTINGS.video_settings_file_path, &file_length);

		if (yaml_file_data != NULL)
		{
			void* hashmap = hashmap_new(20, 2);
			hashmap_read_yaml(hashmap, yaml_file_data, file_length);
			free(yaml_file_data);

			struct hashmap_multi_type* value;

			if (value = hashmap_get_value(hashmap, "gui_scale")) if (value->type == HASHMAP_VALUE_INT)
				SETTINGS.video.gui_scale = (uint32_t)value->data._int;

			if (value = hashmap_get_value(hashmap, "fov")) if (value->type == HASHMAP_VALUE_INT)
				SETTINGS.video.renderer.fov = (uint32_t) value->data._int;

			if (value = hashmap_get_value(hashmap, "render_distance")) if (value->type == HASHMAP_VALUE_INT)
				SETTINGS.video.renderer.render_distance = (uint32_t) value->data._int;

			if (value = hashmap_get_value(hashmap, "backend_index")) if (value->type == HASHMAP_VALUE_INT)
				SETTINGS.video.renderer.backend_index = (uint32_t)value->data._int;

			if (value = hashmap_get_value(hashmap, "backend_device_index")) if (value->type == HASHMAP_VALUE_INT)
				SETTINGS.video.renderer.backend_device_index = (uint32_t)value->data._int;

			if (value = hashmap_get_value(hashmap, "fps")) if (value->type == HASHMAP_VALUE_INT)
				SETTINGS.video.renderer.fps = (uint32_t)value->data._int;

			if (value = hashmap_get_value(hashmap, "vsync")) if (value->type == HASHMAP_VALUE_BOOL)
				SETTINGS.video.renderer.vsync = value->data._bool;

			if (value = hashmap_get_value(hashmap, "max_mipmap_level_count")) if (value->type == HASHMAP_VALUE_INT)
				SETTINGS.video.renderer.max_mipmap_level_count = (uint32_t) value->data._int;

			if (value = hashmap_get_value(hashmap, "order_create_new_destroy_old")) if (value->type == HASHMAP_VALUE_BOOL)
				SETTINGS.video.renderer.order_create_new_destroy_old = value->data._bool;

			hashmap_delete(hashmap);
		}
		else
		{
			minec_client_log_info(client, "[SETTINGS] Could not read from file %s", SETTINGS.video_settings_file_path);
			minec_client_log_debug_l(client, "file_load(%s, &file_length) failed", SETTINGS.video_settings_file_path);
		}
	}

	//resourcepacks
	{
		size_t active_file_length, inactive_file_length;
		void* active_file_data = file_load(SETTINGS.active_resourcepack_file_path, &active_file_length);
		void* inactive_file_data = file_load(SETTINGS.inactive_resourcepack_file_path, &inactive_file_length);
	
		if (active_file_data != NULL)
		{
			arraylist_string_read_file_data(SETTINGS.active_resourcepack_paths_arraylist, active_file_data, active_file_length);
			free(active_file_data);
		}
		else
		{
			minec_client_log_info(client, "[SETTINGS] Could not read from file %s", SETTINGS.active_resourcepack_file_path);
			minec_client_log_debug_l(client, "file_load(%s, &file_length) failed", SETTINGS.active_resourcepack_file_path);
		}

		if (inactive_file_data != NULL)
		{
			arraylist_string_read_file_data(SETTINGS.inactive_resourcepack_paths_arraylist, inactive_file_data, inactive_file_length);
			free(inactive_file_data);
		}
		else
		{
			minec_client_log_info(client, "[SETTINGS] Could not read from file %s", SETTINGS.inactive_resourcepack_file_path);
			minec_client_log_debug_l(client, "file_load(%s, &file_length) failed", SETTINGS.inactive_resourcepack_file_path);
		}
	}

}

void settings_save(struct minec_client* client)
{
	//video settings
	{
		void* hashmap = hashmap_new(20, 2);
		
		//frontend
		hashmap_set_value(hashmap, "gui_scale",						&SETTINGS.video.gui_scale,								HASHMAP_VALUE_INT);
		hashmap_set_value(hashmap, "fov",							&SETTINGS.video.renderer.fov,							HASHMAP_VALUE_INT);
		hashmap_set_value(hashmap, "render_distance",				&SETTINGS.video.renderer.render_distance,				HASHMAP_VALUE_INT);
		hashmap_set_value(hashmap, "backend_index",					&SETTINGS.video.renderer.backend_index,					HASHMAP_VALUE_INT);
		hashmap_set_value(hashmap, "backend_device_index",			&SETTINGS.video.renderer.backend_device_index,			HASHMAP_VALUE_INT);
		hashmap_set_value(hashmap, "fps",							&SETTINGS.video.renderer.fps,							HASHMAP_VALUE_INT);
		hashmap_set_value(hashmap, "vsync",							&SETTINGS.video.renderer.vsync,							HASHMAP_VALUE_BOOL);
		hashmap_set_value(hashmap, "max_mipmap_level_count",		&SETTINGS.video.renderer.max_mipmap_level_count,		HASHMAP_VALUE_INT);
		hashmap_set_value(hashmap, "order_create_new_destroy_old",	&SETTINGS.video.renderer.order_create_new_destroy_old,	HASHMAP_VALUE_BOOL);

		size_t file_length;
		uint8_t* yaml_file_data = hashmap_write_yaml(hashmap, &file_length);

		if (yaml_file_data != NULL)
		{
			if (file_save(SETTINGS.video_settings_file_path, yaml_file_data, file_length) == false)
			{
				minec_client_log_info(client, "[SETTINGS] Could not write to file %s", SETTINGS.video_settings_file_path);
				minec_client_log_debug_l(client, "file_save(%s, &file_length) failed", SETTINGS.video_settings_file_path);
			}
			free(yaml_file_data);
		}

		hashmap_delete(hashmap);
	}
	
	//resourcepacks
	{
		size_t active_file_length, inactive_file_length; 
		uint8_t* active_file_data;
		uint8_t* inactive_file_data;

		if (active_file_data = arraylist_string_write_file_data(SETTINGS.active_resourcepack_paths_arraylist, &active_file_length))
		{
			if (file_save(SETTINGS.active_resourcepack_file_path, active_file_data, active_file_length) == false)
			{
				minec_client_log_info(client, "[SETTINGS] Could not write to file %s", SETTINGS.active_resourcepack_file_path);
				minec_client_log_debug_l(client, "file_save(%s, &file_length) failed", SETTINGS.active_resourcepack_file_path);
			}
			free(active_file_data);
		}

		if (inactive_file_data = arraylist_string_write_file_data(SETTINGS.inactive_resourcepack_paths_arraylist, &inactive_file_length))
		{
			if (file_save(SETTINGS.inactive_resourcepack_file_path, inactive_file_data, inactive_file_length) == false)
			{
				minec_client_log_info(client, "[SETTINGS] Could not write to file %s", SETTINGS.inactive_resourcepack_file_path);
				minec_client_log_debug_l(client, "file_save(%s, &file_length) failed", SETTINGS.inactive_resourcepack_file_path);
			}
			free(inactive_file_data);
		}
	}

}