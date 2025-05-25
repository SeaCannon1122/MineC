#include <minec_client.h>
#include "backend/backend.h"

void rendering_thread_function(struct minec_client* client);

uint32_t _renderer_backend_base_device_create(
	struct minec_client* client,
	uint32_t backend_index,
	struct renderer_backend_base_state* base,
	struct renderer_backend_device_state* device
)
{
	uint32_t result;

	if ((result = client->renderer.backend.global.interfaces[backend_index].base_create(client, &base->base, &base->device_count, &base->device_infos)) != MINEC_CLIENT_SUCCESS) return result;

	for (int32_t i = -1; ; i++)
	{
		if (i == (int32_t)base->device_count)
		{
			client->renderer.backend.global.interfaces[backend_index].base_destroy(client, &base->base);
			return MINEC_CLIENT_ERROR;
		}
		else if (i == -1)
		{
			if (base->device_index < base->device_count)
			{
				if ((result = client->renderer.backend.global.interfaces[backend_index].device_create(client, &base->base, &device->device, base->device_index, device->fps)) == MINEC_CLIENT_SUCCESS)
				{
					break;
				}
				else if (result == MINEC_CLIENT_ERROR_OUT_OF_MEMORY)
				{
					client->renderer.backend.global.interfaces[backend_index].base_destroy(client, &base->base);
					return MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
				}
			}
		}
		else
		{
			if ((result = client->renderer.backend.global.interfaces[backend_index].device_create(client, &base->base, &device->device, (uint32_t)i, device->fps)) == MINEC_CLIENT_SUCCESS)
			{
				base->device_index = (uint32_t)i;
				break;
			}
			else if (result == MINEC_CLIENT_ERROR_OUT_OF_MEMORY)
			{
				client->renderer.backend.global.interfaces[backend_index].base_destroy(client, &base->base);
				return MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
			}
		}
	}

	return MINEC_CLIENT_SUCCESS;
}

void _renderer_backend_base_device_destroy(struct minec_client* client)
{
	client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].device_destroy(client, &client->renderer.backend.base.base, &client->renderer.backend.device.device);
	client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].base_destroy(client, &client->renderer.backend.base.base);
}

uint32_t _renderer_backend_load_base_device_create(
	struct minec_client* client,
	struct renderer_backend_global_state* global,
	struct renderer_backend_base_state* base,
	struct renderer_backend_device_state* device
)
{
	uint32_t result = MINEC_CLIENT_SUCCESS;

	bool
		file_copied = false,
		library_loaded = false
	;

	uint8_t backend_library_temp_name[sizeof("tempx_") + sizeof(MINEC_CLIENT_RENDERER_BACKEND_LIBRARY_NAME) - 1];
	snprintf(backend_library_temp_name, sizeof(backend_library_temp_name), (client->renderer.backend_library_load_index % 2 ? "temp1_%s" : "temp0_%s"), MINEC_CLIENT_RENDERER_BACKEND_LIBRARY_NAME);

	if (file_copy(MINEC_CLIENT_RENDERER_BACKEND_LIBRARY_NAME, backend_library_temp_name) != 0)
	{
		renderer_log(client, "Could not copy %s", MINEC_CLIENT_RENDERER_BACKEND_LIBRARY_NAME);
		result = MINEC_CLIENT_ERROR;
	}
	else file_copied = true;

	if (result == MINEC_CLIENT_SUCCESS)
	{
		if ((global->líbrary_handle = dynamic_library_load(backend_library_temp_name, true)) == NULL)
		{
			renderer_log(client, "Could not open %s", backend_library_temp_name);
			result = MINEC_CLIENT_ERROR;
		}
		else library_loaded = true;
	}

	struct renderer_backend_interface* (*get_interfaces_function)(void* window_context, uint32_t* count, uint8_t*** names);

	if (result == MINEC_CLIENT_SUCCESS)
	{
		if ((get_interfaces_function = dynamic_library_get_function(global->líbrary_handle, "renderer_backend_get_interfaces")) == NULL)
		{
			renderer_log(client, "Failed to retrieve 'renderer_backend_get_interfaces' function from backend");
			result = MINEC_CLIENT_ERROR;
		}
		global->interfaces = get_interfaces_function(window_get_context(), &global->backend_count, &global->backend_names);
	}
	
	for (int32_t i = -1; result == MINEC_CLIENT_SUCCESS; i++)
	{
		if (i == (int32_t)global->backend_count)
		{
			renderer_log(client, "Failed to create any renderer backend");
			result = MINEC_CLIENT_ERROR;
		}
		else if (i == -1)
		{
			if (global->backend_index < global->backend_count)
			{
				if ((result = _renderer_backend_base_device_create(client, global->backend_index, base, device)) == MINEC_CLIENT_SUCCESS)
					break;
				else if (result == MINEC_CLIENT_ERROR) result == MINEC_CLIENT_SUCCESS;
			}
		}
		else
		{
			if ((result = _renderer_backend_base_device_create(client, (uint32_t)i, base, device)) == MINEC_CLIENT_SUCCESS)
			{
				global->backend_index = (uint32_t)i;
				break;
			}
			else if (result == MINEC_CLIENT_ERROR) result == MINEC_CLIENT_SUCCESS;
		}
	}

	if (result != MINEC_CLIENT_SUCCESS && library_loaded) dynamic_library_unload(global->líbrary_handle);
	if (result != MINEC_CLIENT_SUCCESS && file_copied) remove(backend_library_temp_name);

	return result;
}

void _renderer_backend_unload_base_device_destroy(struct minec_client* client)
{
	_renderer_backend_base_device_destroy(client);

	dynamic_library_unload(client->renderer.backend.global.líbrary_handle);

	uint8_t backend_library_temp_name[sizeof("tempx_") + sizeof(MINEC_CLIENT_RENDERER_BACKEND_LIBRARY_NAME) - 1];
	snprintf(backend_library_temp_name, sizeof(backend_library_temp_name), ((client->renderer.backend_library_load_index + 1) % 2 ? "temp1_%s" : "temp0_%s"), MINEC_CLIENT_RENDERER_BACKEND_LIBRARY_NAME);
	remove(backend_library_temp_name);
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
		pixelchar_renderer_create = false,
		backend_base_device_create = false,
		backend_pipelines_resources_create = false
	;

	if ((result = pixelcharRendererCreate(4096, &client->renderer.pixelchar_renderer)) != PIXELCHAR_SUCCESS)
	{
		if (result == PIXELCHAR_ERROR_OUT_OF_MEMORY) result = MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
		else result = MINEC_CLIENT_ERROR;
	}
	else pixelchar_renderer_create = true;
	
	if (result == MINEC_CLIENT_SUCCESS)
	{
		client->renderer.backend.global.backend_index = *backend_index;
		client->renderer.backend.base.device_index = *device_index;
		client->renderer.backend.device.fps = fps;
		client->renderer.backend_library_load_index = 0;

		if ((result = _renderer_backend_load_base_device_create(client, &client->renderer.backend.global, &client->renderer.backend.base, &client->renderer.backend.device)) == MINEC_CLIENT_SUCCESS)
		{
			client->renderer.backend_library_load_index++;
			backend_base_device_create = true;
		}
	}

	if (result == MINEC_CLIENT_SUCCESS)
	{
		if ((result = client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].pipelines_resources_create(client)) == MINEC_CLIENT_SUCCESS)
			backend_pipelines_resources_create = true;
	}

	if (result == MINEC_CLIENT_SUCCESS)
	{
		*backend_index = client->renderer.backend.global.backend_index;
		*backend_count = client->renderer.backend.global.backend_count;
		*backend_names = client->renderer.backend.global.backend_names;
		*device_index = client->renderer.backend.base.device_index;
		*device_count = client->renderer.backend.base.device_count;
		*device_infos = client->renderer.backend.base.device_infos;

		client->renderer.backend.pipelines_resources.created = true;

		mutex_create(&client->renderer.thread_mutex);
		atomic_init(&client->renderer.thread_should_close);
		bool should_close_bool = false;
		atomic_store_(bool, &client->renderer.thread_should_close, &should_close_bool);
		client->renderer.thread_handle = create_thread(rendering_thread_function, client);
	}
	
	if (result != MINEC_CLIENT_SUCCESS && backend_pipelines_resources_create) client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].pipelines_resources_destroy(client);
	if (result != MINEC_CLIENT_SUCCESS && backend_base_device_create) _renderer_backend_unload_base_device_destroy(client);
	if (result != MINEC_CLIENT_SUCCESS && pixelchar_renderer_create) pixelcharRendererDestroy(client->renderer.pixelchar_renderer);

	return result;
}

void renderer_destroy(struct minec_client* client)
{
	bool should_close_bool = true;
	atomic_store_(bool, &client->renderer.thread_should_close, &should_close_bool);
	join_thread(client->renderer.thread_handle);
	atomic_deinit(&client->renderer.thread_should_close);
	mutex_destroy(&client->renderer.thread_mutex);

	if (client->renderer.backend.pipelines_resources.created == true) client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].pipelines_resources_destroy(client);
	_renderer_backend_unload_base_device_destroy(client);

	pixelcharRendererDestroy(client->renderer.pixelchar_renderer);
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
	mutex_lock(&client->renderer.thread_mutex);

	uint32_t result = MINEC_CLIENT_SUCCESS;

	struct renderer_backend_global_state global;
	struct renderer_backend_base_state base;
	struct renderer_backend_device_state device;

	global.backend_index = client->renderer.backend.global.backend_index;
	base.device_index = client->renderer.backend.base.device_index;
	device.fps = client->renderer.backend.device.fps;

	if ((result = _renderer_backend_load_base_device_create(client, &global, &base, &device)) == MINEC_CLIENT_SUCCESS)
	{
		client->renderer.backend_library_load_index++;

		client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].pipelines_resources_destroy(client);
		_renderer_backend_unload_base_device_destroy(client);

		client->renderer.backend.global = global;
		client->renderer.backend.base = base;
		client->renderer.backend.device = device;

		if ((result = client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].pipelines_resources_create(client)) != MINEC_CLIENT_SUCCESS)
		{
			client->renderer.backend.pipelines_resources.created = false;
		}

		*backend_index = global.backend_index;
		*backend_count = global.backend_count;
		*backend_names = global.backend_names;
		*device_index = base.device_index;
		*device_count = base.device_count;
		*device_infos = base.device_infos;
	}

	mutex_unlock(&client->renderer.thread_mutex);

	if (result == MINEC_CLIENT_ERROR) renderer_log(client, "Could not reload backend");
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
	mutex_lock(&client->renderer.thread_mutex);

	uint32_t result = MINEC_CLIENT_SUCCESS;

	struct renderer_backend_base_state base;
	struct renderer_backend_device_state device;

	base.device_index = 0;
	device.fps = client->renderer.backend.device.fps;

	if ((result = _renderer_backend_base_device_create(client, backend_index, &base, &device)) == MINEC_CLIENT_SUCCESS)
	{
		client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].pipelines_resources_destroy(client);
		_renderer_backend_base_device_destroy(client);

		client->renderer.backend.global.backend_index = backend_index;
		client->renderer.backend.base = base;
		client->renderer.backend.device = device;

		if ((result = client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].pipelines_resources_create(client)) != MINEC_CLIENT_SUCCESS)
		{
			client->renderer.backend.pipelines_resources.created = false;
		}

		*device_index = base.device_index;
		*device_count = base.device_count;
		*device_infos = base.device_infos;

	}

	mutex_unlock(&client->renderer.thread_mutex);

	if (result == MINEC_CLIENT_ERROR) renderer_log(client, "Could not switch backend");
	return result;
}

uint32_t renderer_switch_backend_device(struct minec_client* client, uint32_t device_index)
{
	mutex_lock(&client->renderer.thread_mutex);

	uint32_t result = MINEC_CLIENT_SUCCESS;

	struct renderer_backend_device_state device;
	device.fps = client->renderer.backend.device.fps;

	result = client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].device_create(
		client,
		&client->renderer.backend.base.base,
		&device.device,
		device_index,
		client->renderer.backend.device.fps
	);

	if (result == MINEC_CLIENT_SUCCESS)
	{
		client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].pipelines_resources_destroy(client);
		client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].device_destroy(
			client,
			&client->renderer.backend.base.base,
			&client->renderer.backend.device.device
		);

		client->renderer.backend.device = device;

		if ((result = client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].pipelines_resources_create(client)) != MINEC_CLIENT_SUCCESS)
		{
			client->renderer.backend.pipelines_resources.created = false;
		}
	}
	
	mutex_unlock(&client->renderer.thread_mutex);

	if (result == MINEC_CLIENT_ERROR) renderer_log(client, "Could not switch device");
	return result;
}

uint32_t renderer_set_target_fps(struct minec_client* client, uint32_t fps)
{
	mutex_lock(&client->renderer.thread_mutex);
	uint32_t result = client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].set_fps(client, fps);
	mutex_unlock(&client->renderer.thread_mutex);

	return result;
}