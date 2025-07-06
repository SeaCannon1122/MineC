#include "renderer_internal.h"

struct renderer_backend_api
{
	uint32_t(*create)(struct minec_client* client);
	void (*destroy)(struct minec_client* client);
};

static uint8_t* renderer_backend_names[] = {
#ifdef MINEC_CLIENT_RENDERER_BACKEND_OPENGL
	"OpenGL",
#endif
};

static struct renderer_backend_api renderer_backend_apis[] = {
#ifdef MINEC_CLIENT_RENDERER_BACKEND_OPENGL
	{
		renderer_backend_opengl_create,
		renderer_backend_opengl_destroy
	},
#endif
};

static const uint32_t RENDERER_BACKEND_COUNT_c = sizeof(renderer_backend_names) / sizeof(renderer_backend_names[0]);

#define client_to_backend client->renderer.state->public.info.state.backend

uint32_t renderer_backend_initialize(struct minec_client* client)
{
	// assumes RENDERER_BACKEND_COUNT_c <= RENDRER_MAX_BACKEND_COUNT
	for (uint32_t i = 0; i < RENDERER_BACKEND_COUNT_c; i++) snprintf(client_to_backend.backend_infos[i].name, sizeof(client_to_backend.backend_infos[i].name), renderer_backend_names[i]);

	return MINEC_CLIENT_SUCCESS;
}

void renderer_backend_deinitialize(struct minec_client* client)
{

}