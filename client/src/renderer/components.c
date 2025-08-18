#include <minec_client.h>

uint32_t renderer_components_create(struct minec_client* client)
{
	uint32_t result = MINEC_CLIENT_SUCCESS;

	bool
		pixelchar_created = false,
		menu_created = false,
		backend_created = false	  
	;

	if (result == MINEC_CLIENT_SUCCESS)
		if ((result = renderer_component_pixelchar_create(client)) == MINEC_CLIENT_SUCCESS) pixelchar_created = true;
	if (result == MINEC_CLIENT_SUCCESS)
		if ((result = renderer_component_menu_create(client)) == MINEC_CLIENT_SUCCESS) menu_created = true;
	if (result == MINEC_CLIENT_SUCCESS)
		if ((result = renderer_backend_create(client)) == MINEC_CLIENT_SUCCESS) backend_created = true;

	if (result != MINEC_CLIENT_SUCCESS)
	{
		if (backend_created) renderer_backend_destroy(client);
		if (menu_created) renderer_component_menu_destroy(client);
		if (pixelchar_created) renderer_component_pixelchar_destroy(client);
	}

	return result;
}

void renderer_components_destroy(struct minec_client* client)
{
	renderer_backend_destroy(client);
	renderer_component_menu_destroy(client);
	renderer_component_pixelchar_destroy(client);
}

void renderer_reload_assets(struct minec_client* client)
{

}

void renderer_frame_process_actions(struct minec_client* client)
{
	struct renderer_action_state action_state;
	RENDERER_ACCESS_ACTION_STATE(
		action_state = RENDERER.public.action;
		renderer_reset_action_state(client);
	);

	if (action_state.restart)
	{
		renderer_components_destroy(client);
		if (RENDERER_CRASHING) return;
		if (renderer_components_create(client) != MINEC_CLIENT_SUCCESS) { RENDERER_CRASHING = true; return; }
	}

	if (action_state.reload_assets)
	{
		renderer_reload_assets(client);
		if (RENDERER_CRASHING) return;
	}

	if (action_state.order_create_new_destroy_old.change)
	{
		RENDERER.settings.order_create_new_destroy_old = action_state.order_create_new_destroy_old.value;
	}
	if (action_state.fov.change)
	{
		if (action_state.fov.value >= RENDERER_MIN_FOV && action_state.fov.value <= RENDERER_MAX_FOV) RENDERER.settings.fov = action_state.fov.value;
	}
	if (action_state.render_distance.change)
	{
		if (action_state.render_distance.value >= RENDERER_MIN_RENDER_DISTANCE && action_state.render_distance.value <= RENDERER_MAX_RENDER_DISTANCE) RENDERER.settings.render_distance = action_state.render_distance.value;
	}

	if (action_state.backend_device_index.change)	renderer_backend_switch_device(client, action_state.backend_device_index.value);
	if (RENDERER_CRASHING) return;

	if (action_state.backend_index.change)			renderer_backend_switch(client, action_state.backend_index.value);
	if (RENDERER_CRASHING) return;

	if (action_state.vsync.change)					renderer_backend_set_vsync(client, action_state.vsync.value);
	if (RENDERER_CRASHING) return;

	if (action_state.fps.change)					renderer_backend_set_fps(client, action_state.fps.value);
	if (RENDERER_CRASHING) return;

	if (action_state.max_mipmap_level_count.change)	renderer_backend_set_max_mipmap_level_count(client, action_state.max_mipmap_level_count.value);
	if (RENDERER_CRASHING) return;

	RENDERER_ACCESS_PUBLIC_STATE(
		renderer_write_settings_to_settings_state(&RENDERER.public.state.settings, &RENDERER.settings);
		renderer_write_backend_settings_to_settings_state(&RENDERER.public.state.settings, &RENDERER.backend.settings);
		RENDERER.public.state.settings_changed = true;

		if (RENDERER.backend.device_infos_changed)
		{
			RENDERER.public.state.info.backend_device_infos = RENDERER.backend.device_infos;
			RENDERER.public.state.info_changed = true;
		}
	);

}

void renderer_frame(struct minec_client* client)
{
	renderer_frame_process_actions(client);
	if (RENDERER_CRASHING) return;

	int32_t a, b;
	window_get_dimensions(APPLICATION_WINDOW.window_handle, &RENDERER.frame_info.width, &RENDERER.frame_info.height, &a, &b);

	renderer_backend_frame_begin(client);

	renderer_component_menu_frame(client);

	renderer_backend_frame_end(client);
}