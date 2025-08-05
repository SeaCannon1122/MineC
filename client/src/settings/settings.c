#include "minec_client.h"

#include "settings_defines.h"

void settings_create(struct minec_client* client)
{
	uint8_t* path_components[] = { client->data_files_path, VIDEO_SETTINGS_FILE_PATH };
	SETTINGS.video_settings_file_path = malloc_joined_string(path_components, 2);

	SETTINGS.video.frontend.fov = 120;
	SETTINGS.video.frontend.gui_scale = 0;
	SETTINGS.video.frontend.render_distance = 8;

	SETTINGS.video.backend.backend_index = 0;
	SETTINGS.video.backend.backend_device_index = 0;
	SETTINGS.video.backend.fps = 0;
	SETTINGS.video.backend.vsync = true;
	SETTINGS.video.backend.max_mipmap_level_count = 5;

	SETTINGS.video.other.order_create_new_destroy_old = true;
}

void settings_destroy(struct minec_client* client)
{
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

			//frontend
			if (value = hashmap_get_value(hashmap, "fov")) if (value->type == HASHMAP_VALUE_INT)
				SETTINGS.video.frontend.fov = (uint32_t) value->data._int;

			if (value = hashmap_get_value(hashmap, "gui_scale")) if (value->type == HASHMAP_VALUE_INT)
				SETTINGS.video.frontend.gui_scale = (uint32_t) value->data._int;

			if (value = hashmap_get_value(hashmap, "render_distance")) if (value->type == HASHMAP_VALUE_INT)
				SETTINGS.video.frontend.render_distance = (uint32_t) value->data._int;

			//backend
			if (value = hashmap_get_value(hashmap, "backend_index")) if (value->type == HASHMAP_VALUE_INT)
				SETTINGS.video.backend.backend_index = (uint32_t)value->data._int;

			if (value = hashmap_get_value(hashmap, "backend_device_index")) if (value->type == HASHMAP_VALUE_INT)
				SETTINGS.video.backend.backend_device_index = (uint32_t)value->data._int;

			if (value = hashmap_get_value(hashmap, "fps")) if (value->type == HASHMAP_VALUE_INT)
				SETTINGS.video.backend.fps = (uint32_t)value->data._int;

			if (value = hashmap_get_value(hashmap, "vsync")) if (value->type == HASHMAP_VALUE_BOOL)
				SETTINGS.video.backend.vsync = value->data._bool;

			if (value = hashmap_get_value(hashmap, "max_mipmap_level_count")) if (value->type == HASHMAP_VALUE_INT)
				SETTINGS.video.backend.max_mipmap_level_count = (uint32_t) value->data._int;

			//other
			if (value = hashmap_get_value(hashmap, "order_create_new_destroy_old")) if (value->type == HASHMAP_VALUE_BOOL)
				SETTINGS.video.other.order_create_new_destroy_old = value->data._bool;

			hashmap_delete(hashmap);
		}
		else
		{
			minec_client_log_info(client, "[SETTINGS] Could not open %s", SETTINGS.video_settings_file_path);
			minec_client_log_debug_l(client, "file_load(%s, &file_length) failed", SETTINGS.video_settings_file_path);
		}
	}

}

void settings_save(struct minec_client* client)
{
	//video settings
	{
		void* hashmap = hashmap_new(20, 2);
		
		//frontend
		hashmap_set_value(hashmap, "fov",							&SETTINGS.video.frontend.fov,						HASHMAP_VALUE_INT);
		hashmap_set_value(hashmap, "gui_scale",						&SETTINGS.video.frontend.gui_scale,					HASHMAP_VALUE_INT);
		hashmap_set_value(hashmap, "render_distance",				&SETTINGS.video.frontend.render_distance,			HASHMAP_VALUE_INT);
		
		//backend
		hashmap_set_value(hashmap, "backend_index",					&SETTINGS.video.backend.backend_index,				HASHMAP_VALUE_INT);
		hashmap_set_value(hashmap, "backend_device_index",			&SETTINGS.video.backend.backend_device_index,		HASHMAP_VALUE_INT);
		hashmap_set_value(hashmap, "fps",							&SETTINGS.video.backend.fps,							HASHMAP_VALUE_INT);
		hashmap_set_value(hashmap, "vsync",							&SETTINGS.video.backend.vsync,						HASHMAP_VALUE_BOOL);
		hashmap_set_value(hashmap, "max_mipmap_level_count",		&SETTINGS.video.backend.max_mipmap_level_count,		HASHMAP_VALUE_INT);

		//other
		hashmap_set_value(hashmap, "order_create_new_destroy_old",	&SETTINGS.video.other.order_create_new_destroy_old,	HASHMAP_VALUE_BOOL);

		size_t file_length;
		uint8_t* yaml_file_data = hashmap_write_yaml(hashmap, &file_length);

		if (yaml_file_data != NULL)
		{
			if (file_save(SETTINGS.video_settings_file_path, yaml_file_data, file_length) == false)
			{

			}
			free(yaml_file_data);
		}

		hashmap_delete(hashmap);
	}
	
}