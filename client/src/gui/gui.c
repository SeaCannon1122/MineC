#include <minec_client.h>

void _gui_state_assets_load(struct minec_client* client)
{
	pixelfont_load(client);
}

void _gui_state_assets_unload(struct minec_client* client)
{
	if (GUI_STATE.font.data) free(GUI_STATE.font.data);
}

void gui_state_reload_assets(struct minec_client* client)
{
	_gui_state_assets_unload(client);
	_gui_state_assets_load(client);
}

uint32_t gui_state_create(struct minec_client* client)
{
	_gui_state_assets_load(client);

	return MINEC_CLIENT_SUCCESS;
}

void gui_state_destroy(struct minec_client* client)
{
	_gui_state_assets_unload(client);
}