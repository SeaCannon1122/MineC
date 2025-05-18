#include <minec_client.h>

#include "backend/backend.h"

uint32_t _renderer_backend_create(
	struct minec_client* client,
	uint32_t backend_index,
	struct renderer_backend_base_state* base,
	struct renderer_backend_device_state* device,
	struct renderer_backend_pipelines_resources_state* pipelines_resources
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

	if ((result = client->renderer.backend.global.interfaces[backend_index].pipelines_resources_create(client, &base->base, &device->device, &pipelines_resources->pipelines_resources)) != MINEC_CLIENT_SUCCESS)
	{
		client->renderer.backend.global.interfaces[backend_index].device_destroy(client, &base->base, &device->device);
		client->renderer.backend.global.interfaces[backend_index].base_destroy(client, &base->base);
		return result;
	}

	return MINEC_CLIENT_SUCCESS;
}

uint32_t _renderer_backend_load_create(
	struct minec_client* client,
	struct renderer_backend_global_state* global,
	struct renderer_backend_base_state* base,
	struct renderer_backend_device_state* device,
	struct renderer_backend_pipelines_resources_state* pipelines_resources
)
{
	uint32_t result;

	uint8_t backend_library_temp_name[sizeof("tempx_") + sizeof(MINEC_CLIENT_RENDERER_BACKEND_LIBRARY_NAME) - 1];
	snprintf(backend_library_temp_name, sizeof(backend_library_temp_name), (client->renderer.backend_library_load_index % 2 ? "temp1_%s" : "temp0_%s"), MINEC_CLIENT_RENDERER_BACKEND_LIBRARY_NAME);

	if (file_copy(MINEC_CLIENT_RENDERER_BACKEND_LIBRARY_NAME, backend_library_temp_name) != 0)
	{
		renderer_log(client, "Could not copy %s", MINEC_CLIENT_RENDERER_BACKEND_LIBRARY_NAME);
		return MINEC_CLIENT_ERROR;
	}

	if ((global->líbrary_handle = dynamic_library_load(backend_library_temp_name, true)) == NULL)
	{
		renderer_log(client, "Could not open %s", backend_library_temp_name);
		remove(backend_library_temp_name);
		return MINEC_CLIENT_ERROR;
	}

	struct renderer_backend_interface* (*get_interfaces_function)(void* window_context, uint32_t* count, uint8_t*** names);

	if ((get_interfaces_function = dynamic_library_get_function(global->líbrary_handle, "renderer_backend_get_interfaces")) == NULL)
	{
		renderer_log(client, "Failed to retrieve 'renderer_backend_get_interfaces' function from backend");
		dynamic_library_unload(global->líbrary_handle);
		remove(backend_library_temp_name);
		return MINEC_CLIENT_ERROR;
	}
	global->interfaces = get_interfaces_function(window_get_context(), &global->backend_count, &global->backend_names);

	for (int32_t i = -1; ; i++)
	{
		if (i == (int32_t)global->backend_count)
		{
			renderer_log(client, "Failed to create any renderer backend");
			dynamic_library_unload(client->renderer.backend.global.líbrary_handle);
			remove(backend_library_temp_name);
			return MINEC_CLIENT_ERROR;
		}
		else if (i == -1)
		{
			if (global->backend_index < global->backend_count)
			{
				if ((result = _renderer_backend_create(client, global->backend_index, base, device, pipelines_resources)) == MINEC_CLIENT_SUCCESS)
					break;
				else if (result == MINEC_CLIENT_ERROR_OUT_OF_MEMORY)
				{
					dynamic_library_unload(client->renderer.backend.global.líbrary_handle);
					remove(backend_library_temp_name);
					return MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
				}
			}
		}
		else
		{
			if ((result = _renderer_backend_create(client, (uint32_t)i, base, device, pipelines_resources)) == MINEC_CLIENT_SUCCESS)
			{
				global->backend_index = (uint32_t)i;
				break;
			}
			else if (result == MINEC_CLIENT_ERROR_OUT_OF_MEMORY)
			{
				dynamic_library_unload(client->renderer.backend.global.líbrary_handle);
				remove(backend_library_temp_name);
				return MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
			}
		}
	}

	client->renderer.backend_library_load_index++;
	return MINEC_CLIENT_SUCCESS;
}

void _renderer_backend_destroy(struct minec_client* client)
{
	client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].pipelines_resources_destroy(client, &client->renderer.backend.base.base, &client->renderer.backend.device.device, &client->renderer.backend.pipelines_resources.pipelines_resources);
	client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].device_destroy(client, &client->renderer.backend.base.base, &client->renderer.backend.device.device);
	client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].base_destroy(client, &client->renderer.backend.base.base);
}

void _renderer_backend_unload_destroy(struct minec_client* client)
{
	_renderer_backend_destroy(client);

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
	uint32_t result;

	if ((result = pixelcharRendererCreate(4096, &client->renderer.pixelchar_renderer)) != PIXELCHAR_SUCCESS)
	{
		if (result == PIXELCHAR_ERROR_OUT_OF_MEMORY) return MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
		else return MINEC_CLIENT_ERROR;
	}

	client->renderer.backend.global.backend_index = *backend_index;
	client->renderer.backend.base.device_index= *device_index;
	client->renderer.backend.device.fps = fps;
	client->renderer.backend_library_load_index = 0;

	if ((result = _renderer_backend_load_create(client, &client->renderer.backend.global, &client->renderer.backend.base, &client->renderer.backend.device, &client->renderer.backend.pipelines_resources)) != MINEC_CLIENT_SUCCESS) return result;

	*backend_index = client->renderer.backend.global.backend_index;
	*backend_count = client->renderer.backend.global.backend_count;
	*backend_names = client->renderer.backend.global.backend_names;
	*device_index = client->renderer.backend.base.device_index;
	*device_count = client->renderer.backend.base.device_count;
	*device_infos = client->renderer.backend.base.device_infos;

	return MINEC_CLIENT_SUCCESS;
}

void renderer_destroy(struct minec_client* client)
{
	_renderer_backend_unload_destroy(client);

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
	uint32_t result;

	struct renderer_backend_global_state global;
	struct renderer_backend_base_state base;
	struct renderer_backend_device_state device;
	struct renderer_backend_pipelines_resources_state pipelines_resources;

	global.backend_index = client->renderer.backend.global.backend_index;
	base.device_index = client->renderer.backend.base.device_index;
	device.fps = client->renderer.backend.device.fps;

	if ((result = _renderer_backend_load_create(client, &global, &base, &device, &pipelines_resources)) != MINEC_CLIENT_SUCCESS) return result;

	_renderer_backend_unload_destroy(client);

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

	return MINEC_CLIENT_SUCCESS;
}

uint32_t renderer_switch_backend(
	struct minec_client* client, 
	uint32_t backend_index,
	uint32_t* device_index,
	uint32_t* device_count,
	uint8_t*** device_infos
)
{
	uint32_t result;

	struct renderer_backend_base_state base;
	struct renderer_backend_device_state device;
	struct renderer_backend_pipelines_resources_state pipelines_resources;

	base.device_index = 0;
	device.fps = client->renderer.backend.device.fps;

	if ((result = _renderer_backend_create(client, backend_index, &base, &device, &pipelines_resources)) != MINEC_CLIENT_SUCCESS)
	{
		renderer_log(client, "Failed to create renderer backend");
		return result;
	}

	_renderer_backend_destroy(client);

	client->renderer.backend.global.backend_index = backend_index;
	client->renderer.backend.base = base;
	client->renderer.backend.device = device;
	client->renderer.backend.pipelines_resources  = pipelines_resources;

	return MINEC_CLIENT_SUCCESS;
}

uint32_t renderer_switch_backend_device(struct minec_client* client, uint32_t device_index)
{
	uint32_t result;

	struct renderer_backend_device_state device;
	struct renderer_backend_pipelines_resources_state pipelines_resources;

	if (
		(result = client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].device_create(
			client, 
			&client->renderer.backend.base.base,
			&device.device, 
			device_index, client->renderer.backend.device.fps
		)) != MINEC_CLIENT_SUCCESS
	)
	{
		renderer_log(client, "Failed to create renderer backend device");
		return result;
	}

	if (
		(result = client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].pipelines_resources_create(
			client,
			&client->renderer.backend.base.base,
			&device.device,
			&pipelines_resources.pipelines_resources
		)) != MINEC_CLIENT_SUCCESS
	)
	{
		renderer_log(client, "Failed to create renderer backend pipelines_resources");
		client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].device_destroy(
			client,
			&client->renderer.backend.base.base,
			&device.device
		);
		return result;
	}

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

	return MINEC_CLIENT_SUCCESS;
}

void renderer_set_target_fps(struct minec_client* client, uint32_t fps)
{
	client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].set_fps(client, fps);
}