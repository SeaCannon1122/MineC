#include "minec_client.h"

#include "settings_defines.h"

void settings_create(struct minec_client* client)
{
	client->settings.renderer.backend.backend_index = 0;
	client->settings.renderer.backend.backend_device_index = 0;
	client->settings.renderer.backend.fps = 1000;
	client->settings.renderer.backend.vsync = false;
	client->settings.renderer.backend.max_mipmap_level_count = 5;

	client->settings.renderer.frontend.fov = 120;
	client->settings.renderer.frontend.gui_scale = 0;
	client->settings.renderer.frontend.render_distance = 8;

	client->settings.renderer.other.order_create_new_destroy_old = true;
}

void settings_destroy(struct minec_client* client)
{
	
}

void settings_load(struct minec_client* client)
{



}

void settings_save(struct minec_client* client)
{

}