#include <minec_client.h>
#include "backend/backend.h"

void rendering_thread_function(struct minec_client* client);

uint32_t _renderer_backend_create(
	struct minec_client* client,
	uint32_t backend_index,
	struct renderer_backend_base_state* base,
	struct renderer_backend_device_state* device,
	struct renderer_backend_pipelines_resources_state* pipelines_resources
)
{
	uint32_t result = MINEC_CLIENT_SUCCESS;

	bool
		base_created = false,
		device_created = false,
		pipelines_resources_created = false
	;

	uint32_t device_index = base->device_index;

	if ((result = client->renderer.backend.global.interfaces[backend_index].base_create(client, &base->base, &base->device_count, &base->device_infos)) == MINEC_CLIENT_SUCCESS) base_created = true;

	for (int32_t i = -1; result == MINEC_CLIENT_SUCCESS && device_created == false; i++)
	{
		if (i == (int32_t)base->device_count) result = MINEC_CLIENT_ERROR;
		else if (i == -1)
		{
			if (device_index < base->device_count)
			{
				if ((result = client->renderer.backend.global.interfaces[backend_index].device_create(client, &base->base, &device->device, device_index, device->fps)) == MINEC_CLIENT_SUCCESS) device_created = true;
				else if (result == MINEC_CLIENT_ERROR) result = MINEC_CLIENT_SUCCESS;
			}
		}
		else
		{
			if ((result = client->renderer.backend.global.interfaces[backend_index].device_create(client, &base->base, &device->device, (uint32_t)i, device->fps)) == MINEC_CLIENT_SUCCESS)
			{
				device_index = (uint32_t)i;
				device_created = true;
			}
			else if (result == MINEC_CLIENT_ERROR) result = MINEC_CLIENT_SUCCESS;
		}
	}

	if (result == MINEC_CLIENT_SUCCESS && device_created == false) result = MINEC_CLIENT_ERROR;
	if (result == MINEC_CLIENT_SUCCESS) if ((result = client->renderer.backend.global.interfaces[backend_index].pipelines_resources_create(client, &base->base, &device->device, &pipelines_resources->pipelines_resources, pipelines_resources->pcr_backend_index)) == MINEC_CLIENT_SUCCESS) pipelines_resources_created = true;

	if (result != MINEC_CLIENT_SUCCESS)
	{
		if (device_created) client->renderer.backend.global.interfaces[backend_index].device_destroy(client, &base->base, &device->device);
		if (base_created) client->renderer.backend.global.interfaces[backend_index].base_destroy(client, &base->base);
	}
	else base->device_index = device_index;
	
	return MINEC_CLIENT_SUCCESS;
}

void _renderer_backend_destroy(
	struct minec_client* client,
	uint32_t backend_index,
	struct renderer_backend_base_state* base,
	struct renderer_backend_device_state* device,
	struct renderer_backend_pipelines_resources_state* pipelines_resources
)
{
	client->renderer.backend.global.interfaces[backend_index].pipelines_resources_destroy(client, &base->base, &device->device, &pipelines_resources->pipelines_resources);
	client->renderer.backend.global.interfaces[backend_index].device_destroy(client, &base->base, &device->device);
	client->renderer.backend.global.interfaces[backend_index].base_destroy(client, &base->base);
}

uint32_t _renderer_backend_load_create(
	struct minec_client* client,
	struct renderer_backend_global_state* global,
	struct renderer_backend_base_state* base,
	struct renderer_backend_device_state* device,
	struct renderer_backend_pipelines_resources_state* pipelines_resources
)
{
	uint32_t result = MINEC_CLIENT_SUCCESS;

	bool
		file_copied = false,
		library_loaded = false
	;

	if (file_copy(client->renderer.backend_library_paths[2], client->renderer.backend_library_paths[global->library_load_index]) != 0)
	{
		minec_client_log_error(client, "[RENDERER] Could not copy %s to %s", client->renderer.backend_library_paths[2], client->renderer.backend_library_paths[global->library_load_index]);
		minec_client_log_debug_error(client, "'file_copy(%s, %s)' failed", client->renderer.backend_library_paths[2], client->renderer.backend_library_paths[global->library_load_index]);
		result = MINEC_CLIENT_ERROR;
	}
	else file_copied = true;

	if (result == MINEC_CLIENT_SUCCESS)
	{
		if ((global->líbrary_handle = dynamic_library_load(client->renderer.backend_library_paths[global->library_load_index], true)) == NULL)
		{
			minec_client_log_error(client, "[RENDERER] Could not fully process dynamic library %s. May be corrupted or out of date version.", client->renderer.backend_library_paths[global->library_load_index]);
			minec_client_log_debug_error(client, "'dynamic_library_load(%s)' failed", client->renderer.backend_library_paths[global->library_load_index]);
			result = MINEC_CLIENT_ERROR;
		}
		else library_loaded = true;
	}

	struct renderer_backend_interface* (*get_interfaces_function)(void* window_context, uint32_t* count, uint8_t*** names);

	if (result == MINEC_CLIENT_SUCCESS)
	{
		if ((get_interfaces_function = dynamic_library_get_function(global->líbrary_handle, "renderer_backend_get_interfaces")) == NULL)
		{
			minec_client_log_error(client, "[RENDERER] Could not fully process dynamic library %s. May be corrupted or out of date version.", client->renderer.backend_library_paths[global->library_load_index]);
			minec_client_log_debug_error(client, "'dynamic_library_get_function(global->líbrary_handle, \"renderer_backend_get_interfaces\")' failed");
			result = MINEC_CLIENT_ERROR;
		}
		global->interfaces = get_interfaces_function(window_get_context(), &global->backend_count, &global->backend_names);
	}
	
	for (int32_t i = -1; result == MINEC_CLIENT_SUCCESS; i++)
	{
		if (i == (int32_t)global->backend_count)
		{
			minec_client_log_error(client, "[RENDERER] Failed to create any renderer backend sucessfully");
			result = MINEC_CLIENT_ERROR;
		}
		else if (i == -1)
		{
			if (global->backend_index < global->backend_count)
			{
				if ((result = _renderer_backend_create(client, global->backend_index, base, device, pipelines_resources)) == MINEC_CLIENT_SUCCESS)
					break;
				else if (result == MINEC_CLIENT_ERROR) result == MINEC_CLIENT_SUCCESS;
			}
		}
		else
		{
			if ((result = _renderer_backend_create(client, (uint32_t)i, base, device, pipelines_resources)) == MINEC_CLIENT_SUCCESS)
			{
				global->backend_index = (uint32_t)i;
				break;
			}
			else if (result == MINEC_CLIENT_ERROR) result == MINEC_CLIENT_SUCCESS;
		}
	}

	if (result != MINEC_CLIENT_SUCCESS && library_loaded) dynamic_library_unload(global->líbrary_handle);
	if (result != MINEC_CLIENT_SUCCESS && file_copied) remove(client->renderer.backend_library_paths[global->library_load_index]);

	return result;
}

void _renderer_backend_unload_destroy(
	struct minec_client* client,
	struct renderer_backend_global_state* global,
	struct renderer_backend_base_state* base,
	struct renderer_backend_device_state* device,
	struct renderer_backend_pipelines_resources_state* pipelines_resources
)
{
	_renderer_backend_destroy(client, global->backend_index, base, device, pipelines_resources);

	dynamic_library_unload(global->líbrary_handle);
	remove(client->renderer.backend_library_paths[global->library_load_index]);
}

uint32_t renderer_create(
	struct minec_client* client,
	uint32_t* backend_index,
	uint32_t* backend_count,
	uint8_t*** backend_names,
	uint32_t* device_index,
	uint32_t* device_count,
	uint8_t*** device_infos,
	uint32_t fps
)
{
	uint32_t result = MINEC_CLIENT_SUCCESS;

	bool
		backend_library_names_memory = false,
		pixelchar_renderer_create = false,
		backend_loaded_created = false
	;

	uint8_t* backend_library_path_parts[] = {
		client->data_files_path,
		"temp0_",
		MINEC_CLIENT_RENDERER_BACKEND_LIBRARY_NAME
	};

	if (result == MINEC_CLIENT_SUCCESS)
	{
		if ((client->renderer.backend_library_paths[0] = s_alloc_joined_string(client->static_alloc, backend_library_path_parts, 3)) == NULL) result = PIXELCHAR_ERROR_OUT_OF_MEMORY;
		else
		{
			backend_library_path_parts[1] = "temp1_";
			if ((client->renderer.backend_library_paths[1] = s_alloc_joined_string(client->static_alloc, backend_library_path_parts, 3)) == NULL)
			{
				s_free(client->static_alloc, client->renderer.backend_library_paths[0]);
				result = PIXELCHAR_ERROR_OUT_OF_MEMORY;
			}
			else
			{
				backend_library_path_parts[1] = MINEC_CLIENT_RENDERER_BACKEND_LIBRARY_NAME;
				if ((client->renderer.backend_library_paths[2] = s_alloc_joined_string(client->static_alloc, backend_library_path_parts, 2)) == NULL)
				{
					s_free(client->static_alloc, client->renderer.backend_library_paths[0]);
					s_free(client->static_alloc, client->renderer.backend_library_paths[1]);
					result = PIXELCHAR_ERROR_OUT_OF_MEMORY;
				}
				else backend_library_names_memory = true;
			}
		}		
	}

	if (result == MINEC_CLIENT_SUCCESS)
	{
		if ((result = pixelcharRendererCreate(4096, &client->renderer.pixelchar_renderer)) != PIXELCHAR_SUCCESS) result = MINEC_CLIENT_ERROR;
		else
		{
			result = MINEC_CLIENT_SUCCESS;
			pixelchar_renderer_create = true;
		}
	}

	if (result == MINEC_CLIENT_SUCCESS)
	{
		uint32_t pixelchar_font_count = (client->resource_index.pixelchar_font_count < PIXELCHAR_RENDERER_MAX_FONT_COUNT ? client->resource_index.pixelchar_font_count : PIXELCHAR_RENDERER_MAX_FONT_COUNT);

		for (uint32_t i = 0; i < pixelchar_font_count; i++)
		{
			PixelcharFont font;
			uint32_t pc_result = pixelcharFontCreate(client->resource_index.pixelchar_fonts[i].font_file_data, client->resource_index.pixelchar_fonts[i].font_file_data_size, &font);
			if (result == PIXELCHAR_SUCCESS)
			{
				pixelcharRendererBindFont(client->renderer.pixelchar_renderer, font, i);
				pixelcharFontDestroy(font);
			}
		}
	}
	
	if (result == MINEC_CLIENT_SUCCESS)
	{
		client->renderer.backend.global.library_load_index = 0;
		client->renderer.backend.global.backend_index = *backend_index;
		client->renderer.backend.base.device_index = *device_index;
		client->renderer.backend.device.fps = fps;
		client->renderer.backend.pipelines_resources.pcr_backend_index = 0;

		if ((result = _renderer_backend_load_create(client, &client->renderer.backend.global, &client->renderer.backend.base, &client->renderer.backend.device, &client->renderer.backend.pipelines_resources)) == MINEC_CLIENT_SUCCESS)
		{
			backend_loaded_created = true;
		}
	}

	if (result == MINEC_CLIENT_SUCCESS)
	{
		*backend_index = client->renderer.backend.global.backend_index;
		*backend_count = client->renderer.backend.global.backend_count;
		*backend_names = client->renderer.backend.global.backend_names;
		*device_index = client->renderer.backend.base.device_index;
		*device_count = client->renderer.backend.base.device_count;
		*device_infos = client->renderer.backend.base.device_infos;

		client->renderer.thread_state.frame_info.time = 0.f;
		client->renderer.thread_state.frame_info.index = 0;

		atomic_init(&client->renderer.request_flag);
		uint32_t request_flag = RENDERER_REQUEST_RENDER;
		atomic_store_(uint32_t, &client->renderer.request_flag, &request_flag);

		mutex_create(&client->renderer.mutex);

		client->renderer.thread_state.handle = create_thread(rendering_thread_function, client);
	}

	if (result != MINEC_CLIENT_SUCCESS && backend_loaded_created) _renderer_backend_unload_destroy(client, &client->renderer.backend.global, &client->renderer.backend.base, &client->renderer.backend.device, &client->renderer.backend.pipelines_resources);
	if (result != MINEC_CLIENT_SUCCESS && pixelchar_renderer_create) pixelcharRendererDestroy(client->renderer.pixelchar_renderer);
	if (result != MINEC_CLIENT_SUCCESS && backend_library_names_memory) for (uint32_t i = 0; i < 3; i++) s_free(client->static_alloc, client->renderer.backend_library_paths[i]);

	return result;
}

void renderer_destroy(struct minec_client* client)
{
	uint32_t request_flag = RENDERER_REQUEST_CLOSE;
	atomic_store_(uint32_t, &client->renderer.request_flag, &request_flag);

	join_thread(client->renderer.thread_state.handle);

	mutex_destroy(&client->renderer.mutex);
	atomic_deinit(&client->renderer.request_flag);

	_renderer_backend_unload_destroy(client, &client->renderer.backend.global, &client->renderer.backend.base, &client->renderer.backend.device, &client->renderer.backend.pipelines_resources);

	pixelcharRendererDestroy(client->renderer.pixelchar_renderer);

	for (uint32_t i = 0; i < 3; i++) s_free(client->static_alloc, client->renderer.backend_library_paths[i]);
}

uint32_t renderer_reload_backend(
	struct minec_client* client,
	uint32_t* backend_index,
	uint32_t* backend_count,
	uint8_t*** backend_names,
	uint32_t* device_index,
	uint32_t* device_count,
	uint8_t*** device_infos
)
{
	uint32_t request_flag = RENDERER_REQUEST_HALT;
	atomic_store_(uint32_t, &client->renderer.request_flag, &request_flag);
	mutex_lock(&client->renderer.mutex);

	uint32_t result = MINEC_CLIENT_SUCCESS;

	struct renderer_backend_global_state global;
	struct renderer_backend_base_state base;
	struct renderer_backend_device_state device;
	struct renderer_backend_pipelines_resources_state pipelines_resources;

	global.library_load_index = (client->renderer.backend.global.library_load_index + 1) % 2;
	global.backend_index = client->renderer.backend.global.backend_index;
	base.device_index = client->renderer.backend.base.device_index;
	device.fps = client->renderer.backend.device.fps;
	pipelines_resources.pcr_backend_index = (client->renderer.backend.pipelines_resources.pcr_backend_index + 1) % 2;

	if ((result = _renderer_backend_load_create(client, &global, &base, &device, &pipelines_resources)) == MINEC_CLIENT_SUCCESS)
	{

		_renderer_backend_unload_destroy(client, &client->renderer.backend.global, &client->renderer.backend.base, &client->renderer.backend.device, &client->renderer.backend.pipelines_resources);

		client->renderer.backend.global = global;
		client->renderer.backend.base = base;
		client->renderer.backend.device = device;
		client->renderer.backend.pipelines_resources = pipelines_resources;

		*backend_index = global.backend_index;
		*backend_count = global.backend_count;
		*backend_names = global.backend_names;
		*device_index = base.device_index;
		*device_count = base.device_count;
		*device_infos = base.device_infos;
	}

	mutex_unlock(&client->renderer.mutex);
	return result;
}

uint32_t renderer_switch_backend(
	struct minec_client* client, 
	uint32_t backend_index,
	uint32_t* device_index,
	uint32_t* device_count,
	uint8_t*** device_infos
)
{
	mutex_lock(&client->renderer.mutex);

	uint32_t result = MINEC_CLIENT_SUCCESS;

	struct renderer_backend_base_state base;
	struct renderer_backend_device_state device;
	struct renderer_backend_pipelines_resources_state pipelines_resources;

	base.device_index = 0;
	device.fps = client->renderer.backend.device.fps;
	pipelines_resources.pcr_backend_index = (client->renderer.backend.pipelines_resources.pcr_backend_index + 1) % 2;

	if ((result = _renderer_backend_create(client, backend_index, &base, &device, &pipelines_resources)) == MINEC_CLIENT_SUCCESS)
	{
		_renderer_backend_destroy(client, client->renderer.backend.global.backend_index, &base, &device, &pipelines_resources);

		client->renderer.backend.global.backend_index = backend_index;
		client->renderer.backend.base = base;
		client->renderer.backend.device = device;
		client->renderer.backend.pipelines_resources = pipelines_resources;

		*device_index = base.device_index;
		*device_count = base.device_count;
		*device_infos = base.device_infos;
	}

	mutex_unlock(&client->renderer.mutex);
	return result;
}

uint32_t renderer_switch_backend_device(struct minec_client* client, uint32_t device_index)
{
	uint32_t request_flag = RENDERER_REQUEST_HALT;
	atomic_store_(uint32_t, &client->renderer.request_flag, &request_flag);
	mutex_lock(&client->renderer.mutex);

	uint32_t result = MINEC_CLIENT_SUCCESS;

	struct renderer_backend_device_state device;
	struct renderer_backend_pipelines_resources_state pipelines_resources;

	device.fps = client->renderer.backend.device.fps;
	pipelines_resources.pcr_backend_index = (client->renderer.backend.pipelines_resources.pcr_backend_index + 1) % 2;

	if ((
		result = client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].device_create(
		client,
		&client->renderer.backend.base.base,
		&device.device,
		device_index,
		client->renderer.backend.device.fps
	)) == MINEC_CLIENT_SUCCESS)
	{
		if ((result = client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].pipelines_resources_create(
			client,
			&client->renderer.backend.base.base,
			&device.device,
			&pipelines_resources.pipelines_resources,
			pipelines_resources.pcr_backend_index
		)) != MINEC_CLIENT_SUCCESS)
		{
			client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].device_destroy(
				client,
				&client->renderer.backend.base.base,
				&device.device
			);
		}
		else
		{
			client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].pipelines_resources_destroy(
				client,
				&client->renderer.backend.base.base,
				&client->renderer.backend.device.device,
				&client->renderer.backend.pipelines_resources.pipelines_resources
			);
			client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].device_destroy(
				client,
				&client->renderer.backend.base.base,
				&client->renderer.backend.device.device
			);

			client->renderer.backend.device = device;
			client->renderer.backend.pipelines_resources = pipelines_resources;
		}
	}
	
	mutex_unlock(&client->renderer.mutex);
	return result;
}

uint32_t renderer_set_target_fps(struct minec_client* client, uint32_t fps)
{
	uint32_t request_flag = RENDERER_REQUEST_HALT;
	atomic_store_(uint32_t, &client->renderer.request_flag, &request_flag);
	mutex_lock(&client->renderer.mutex);

	uint32_t result = client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].set_fps(client, fps);

	mutex_unlock(&client->renderer.mutex);
	return result;
}