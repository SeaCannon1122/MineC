#include <minec_client.h>

#include "backend/backend.h"

uint32_t _renderer_backend_create(
	struct minec_client* client, 
	void** base, 
	void** device, 
	void** pipelines_resources, 
	uint32_t backend_index, 
	uint32_t* device_index,
	uint32_t* device_count, 
	uint8_t*** device_infos, 
	uint32_t fps
)
{
	uint32_t result;

	if ((result = client->renderer.backend_interfaces[backend_index].base_create(client, base, device_count, device_infos)) != MINEC_CLIENT_SUCCESS) return result;

	for (int32_t i = -1; ; i++)
	{
		if (i == (int32_t)*device_count)
		{
			client->renderer.backend_interfaces[backend_index].base_destroy(client, base);
			return MINEC_CLIENT_ERROR;
		}
		else if (i == -1 && *device_index < *device_count)
		{
			if ((result = client->renderer.backend_interfaces[backend_index].device_create(client, base, device, *device_index, fps)) == MINEC_CLIENT_SUCCESS)
			{
				break;
			}
			else if (result == MINEC_CLIENT_ERROR_OUT_OF_MEMORY)
			{
				client->renderer.backend_interfaces[backend_index].base_destroy(client, base);
				return MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
			}
		}
		else
		{
			if ((result = client->renderer.backend_interfaces[backend_index].device_create(client, base, device, i, fps)) == MINEC_CLIENT_SUCCESS)
			{
				*device_index = i;
				break;
			}
			else if (result == MINEC_CLIENT_ERROR_OUT_OF_MEMORY)
			{
				client->renderer.backend_interfaces[backend_index].base_destroy(client, base);
				return MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
			}
		}
	}

	if ((result = client->renderer.backend_interfaces[backend_index].pipelines_resources_create(client, base, device, pipelines_resources)) != MINEC_CLIENT_SUCCESS)
	{
		client->renderer.backend_interfaces[backend_index].device_destroy(client, base, device);
		client->renderer.backend_interfaces[backend_index].base_destroy(client, base);
		return result;
	}

	return MINEC_CLIENT_SUCCESS;
}

void _renderer_backend_destroy(
	struct minec_client* client,
	void** base,
	void** device,
	void** pipelines_resources,
	uint32_t backend_index
)
{
	client->renderer.backend_interfaces[backend_index].pipelines_resources_destroy(client, base, device, pipelines_resources);
	client->renderer.backend_interfaces[backend_index].device_destroy(client, base, device);
	client->renderer.backend_interfaces[backend_index].base_destroy(client, base);
}

uint32_t _renderer_backend_load_create(
	struct minec_client* client,
	void** base,
	void** device,
	void** pipelines_resources,
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

	uint8_t backend_library_temp_name[sizeof("tempx_") + sizeof(MINEC_CLIENT_RENDERER_BACKEND_LIBRARY_NAME) - 1];
	snprintf(backend_library_temp_name, sizeof(backend_library_temp_name), (client->renderer.backend_library_load_index % 2 ? "temp1_%s" : "temp0_%s"), MINEC_CLIENT_RENDERER_BACKEND_LIBRARY_NAME);

	if ((client->renderer.backend_líbrary_handle = dynamic_library_load(MINEC_CLIENT_RENDERER_BACKEND_LIBRARY_NAME, true)) == NULL)
	{
		renderer_log(client, "Could not open %s", MINEC_CLIENT_RENDERER_BACKEND_LIBRARY_NAME);
		return MINEC_CLIENT_ERROR;
	}

	struct renderer_backend_interface* (*get_interfaces_function)(void* window_context, uint32_t* count, uint8_t*** names);
	uint8_t** (*get_names_function)();

	if ((get_interfaces_function = dynamic_library_get_function(client->renderer.backend_líbrary_handle, "renderer_backend_get_interfaces")) == NULL)
	{
		renderer_log(client, "Failed to retrieve 'renderer_backend_get_interfaces' function from backend");
		dynamic_library_unload(client->renderer.backend_líbrary_handle);
		return MINEC_CLIENT_ERROR;
	}
	client->renderer.backend_interfaces = get_interfaces_function(window_get_context(), backend_count, backend_names);

	if ((get_names_function = dynamic_library_get_function(client->renderer.backend_líbrary_handle, "renderer_backend_get_names")) == NULL)
	{
		renderer_log(client, "Failed to retrieve 'renderer_backend_get_names' function from backend");
		dynamic_library_unload(client->renderer.backend_líbrary_handle);
		return MINEC_CLIENT_ERROR;
	}
	client->renderer.backend_interfaces = get_interfaces_function(window_get_context(), backend_count, backend_names);

	for (int32_t i = -1; ; i++)
	{
		if (i == (int32_t)*backend_count)
		{
			renderer_log(client, "Failed to create any renderer backend");
			dynamic_library_unload(client->renderer.backend_líbrary_handle);
			return MINEC_CLIENT_ERROR;
		}
		else if (i == -1 && *backend_index < *backend_count)
		{
			if ((result = _renderer_backend_create(client, base, device, pipelines_resources, *backend_index, device_index, device_count, device_infos, fps)) == MINEC_CLIENT_SUCCESS)
				break;
			else if (result == MINEC_CLIENT_ERROR_OUT_OF_MEMORY)
			{
				dynamic_library_unload(client->renderer.backend_líbrary_handle);
				return MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
			}
		}
		else
		{
			if ((result = _renderer_backend_create(client, base, device, pipelines_resources, (uint32_t)i, device_index, device_count, device_infos, fps)) == MINEC_CLIENT_SUCCESS)
			{
				*backend_index = i;
				break;
			}
			else if (result == MINEC_CLIENT_ERROR_OUT_OF_MEMORY)
			{
				dynamic_library_unload(client->renderer.backend_líbrary_handle);
				return MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
			}
		}
	}

	return MINEC_CLIENT_SUCCESS;
}

void _renderer_backend_unload_destroy(struct minec_client* client)
{
	client->renderer.backend_interfaces[client->renderer.backend_index].pipelines_resources_destroy(client, &client->renderer.backend_base, &client->renderer.backend_device, &client->renderer.backend_pipelines_resources);
	client->renderer.backend_interfaces[client->renderer.backend_index].device_destroy(client, &client->renderer.backend_base, &client->renderer.backend_device);
	client->renderer.backend_interfaces[client->renderer.backend_index].base_destroy(client, &client->renderer.backend_base);

	dynamic_library_unload(client->renderer.backend_líbrary_handle);
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

	if ((result = _renderer_backend_initial_create(client, backend_index, backend_count, backend_names, device_index, device_count, device_infos, fps)) != MINEC_CLIENT_SUCCESS) return result;

	pixelcharRendererCreate(4096, &client->renderer.pixelchar_renderer);

	client->renderer.backend_library_load_index = 0;
	client->renderer.fps = fps;

	return MINEC_CLIENT_SUCCESS;
}

void renderer_destroy(struct minec_client* client)
{
	pixelcharRendererDestroy(client->renderer.pixelchar_renderer);

	_renderer_backend_final_destroy(client);
}

uint32_t renderer_reload_backend(
	struct minec_client* client,
	uint32_t* backend_index,
	uint32_t* backend_count,
	uint8_t*** backend_names,
	uint32_t* device_index,
	uint32_t* device_count,
	uint8_t*** device_infos,
)
{
	_renderer_backend_final_destroy(client);
}

uint32_t renderer_switch_backend(struct minec_client* client, uint32_t backend_index)
{

}

uint32_t renderer_switch_backend_device(struct minec_client* client, uint32_t device)
{

}

void renderer_set_target_fps(struct minec_client* client, uint32_t fps)
{

}