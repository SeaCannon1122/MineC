#include <minec_client.h>

#include "backend/backend.h"

//uint32_t _renderer_backend_create_base_and_device(struct minec_client* client, uint32_t slot_index, uint32_t backend_index, uint32_t* device_index, uint32_t* device_count, uint8_t*** device_infos, uint32_t fps)
//{
//	struct renderer_backend_interface* interfaces = renderer_backend_get_interface(backend_index);
//
//	uint32_t result;
//
//	if ((result = backend_if->base_create(client, slot_index)) != MINEC_CLIENT_SUCCESS) return result;
//
//	*device_infos = backend_if->get_device_infos(client, slot_index, device_count);
//
//	if (*device_index < *device_count) if ((result = backend_if->device_create(client, slot_index, *device_index, fps)) == MINEC_CLIENT_SUCCESS)
//		return MINEC_CLIENT_SUCCESS;
//
//	uint32_t initial_device_index = *device_index;
//
//	for (*device_index = 0; *device_index < *device_count && result != MINEC_CLIENT_ERROR_OUT_OF_MEMORY; *device_index++) if (*device_index != initial_device_index)
//	{
//		if ((result = backend_if->device_create(client, slot_index, *device_index, fps)) == MINEC_CLIENT_SUCCESS)
//			return MINEC_CLIENT_SUCCESS;
//	}
//
//	backend_if->base_destroy(client, slot_index);
//
//	return result;
//}

uint32_t _renderer_backend_initial_create(struct minec_client* client)
{
	uint32_t result;

	if ((client->renderer.backend_líbrary_handle = dynamic_library_load("minec_client_renderer_backend", false)) == NULL)
	{
		renderer_log(client, "Could not open shared/dynamic library 'minec_client_renderer_backend'");
		return MINEC_CLIENT_ERROR;
	}

	return MINEC_CLIENT_SUCCESS;
}

void _renderer_backend_final_destroy(struct minec_client* client)
{
	dynamic_library_unload(client->renderer.backend_líbrary_handle);
}

uint32_t renderer_create(struct minec_client* client)
{
	uint32_t result;

	if ((result = _renderer_backend_initial_create(client)) != MINEC_CLIENT_SUCCESS) return result;

	pixelcharRendererCreate(4096, &client->renderer.pixelchar_renderer);

	return MINEC_CLIENT_SUCCESS;
}

void renderer_destroy(struct minec_client* client)
{
	pixelcharRendererDestroy(client->renderer.pixelchar_renderer);

	_renderer_backend_final_destroy(client);
}