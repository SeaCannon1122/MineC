#include "minec_client.h"

void renderer_create(struct minec_client* client)
{
	pixelchar_renderer_create(&client->renderer_state.pixelchar_renderer, 4096);
}

void renderer_destroy(struct minec_client* client)
{
	pixelchar_renderer_destroy(&client->renderer_state.pixelchar_renderer);
}

void renderer_render(struct minec_client* client)
{

}

void renderer_resize(struct minec_client* client)
{

}

void renderer_reload_resources(struct minec_client* client)
{

}