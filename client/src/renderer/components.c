#include <minec_client.h>

uint32_t _backend_create_device(struct minec_client* client)
{
	uint32_t result;

	for (int32_t i = -1, original_i = RENDERER.settings.backend_device_index; i < (int32_t)RENDERER.backend_device_infos.count; i++, RENDERER.settings.backend_device_index = i)
	{
		if (i == original_i) continue;
		if ((result = renderer_backend_device_create(client, RENDERER.settings.backend_device_index)) == MINEC_CLIENT_SUCCESS)
			return MINEC_CLIENT_SUCCESS;
		else if (result == MINEC_CLIENT_ERROR_BACKEND_CRASHED) return MINEC_CLIENT_ERROR;
	}

	return MINEC_CLIENT_ERROR;
}

uint32_t _backend_create_base_and_device(struct minec_client* client)
{
	for (int32_t i = -1, original_i = RENDERER.settings.backend_index; i < RENDERER_BACKEND_COUNT; i++, RENDERER.settings.backend_index = i)
	{
		if (i == original_i) continue;
		if (renderer_backend_base_create(client, RENDERER.settings.backend_index, APPLICATION_WINDOW.context, APPLICATION_WINDOW.window, &RENDERER.backend_device_infos) == MINEC_CLIENT_SUCCESS)
		{
			if (_backend_create_device(client) == MINEC_CLIENT_SUCCESS) return MINEC_CLIENT_SUCCESS;
			renderer_backend_base_destroy(client);
		}
	}

	return MINEC_CLIENT_ERROR;
}

uint32_t renderer_components_create(struct minec_client* client)
{
	uint32_t result = MINEC_CLIENT_SUCCESS;

	memset(&RENDERER.components, 0, sizeof(RENDERER.components));

	int32_t x, y;
	cwindow_get_dimensions(APPLICATION_WINDOW.window, &RENDERER.frame_info.width, &RENDERER.frame_info.height, &x, &y);

	if (result == MINEC_CLIENT_SUCCESS)
	{
		if (_backend_create_base_and_device(client) == MINEC_CLIENT_SUCCESS)
		{
			RENDERER.components.backend_base_created = true;
			RENDERER.components.backend_device_created = true;
		}
		else result = MINEC_CLIENT_ERROR;	
	}
		
	if (result == MINEC_CLIENT_SUCCESS)
	{
		if (RENDERER.backend_device_infos.infos[RENDERER.settings.backend_device_index].disable_vsync_support == false) RENDERER.settings.vsync = true;
		if (RENDERER.backend_device_infos.infos[RENDERER.settings.backend_device_index].triple_buffering_support == false) RENDERER.settings.triple_buffering = false;

		if (renderer_backend_swapchain_create(client, RENDERER.frame_info.width, RENDERER.frame_info.height, RENDERER.settings.vsync, RENDERER.settings.triple_buffering) == MINEC_CLIENT_SUCCESS)
			RENDERER.components.backend_swapchain_created = true;
		else result = MINEC_CLIENT_ERROR;
	}
		
	if (result != MINEC_CLIENT_SUCCESS)
	{
		if (RENDERER.components.backend_swapchain_created) renderer_backend_swapchain_destroy(client);
		if (RENDERER.components.backend_device_created) renderer_backend_device_destroy(client);
		if (RENDERER.components.backend_base_created) renderer_backend_base_destroy(client);
	}

	return result;
}

void renderer_components_destroy(struct minec_client* client)
{
	if (RENDERER.components.backend_swapchain_created) renderer_backend_swapchain_destroy(client);
	if (RENDERER.components.backend_device_created) renderer_backend_device_destroy(client);
	if (RENDERER.components.backend_base_created) renderer_backend_base_destroy(client);
}

void renderer_reload_assets(struct minec_client* client)
{

}

uint32_t renderer_frame_process_actions(struct minec_client* client)
{
	struct renderer_action_state action_state;
	RENDERER_ACCESS_ACTION_STATE(
		action_state = RENDERER.public.action;
		renderer_reset_action_state(client);
	);

	if (action_state.restart)
	{

	}
	if (action_state.reload_assets)
	{

	}
	if (action_state.fov.change)
	{
		if (action_state.fov.value >= RENDERER_MIN_FOV && action_state.fov.value <= RENDERER_MAX_FOV) RENDERER.settings.fov = action_state.fov.value;
	}
	if (action_state.render_distance.change)
	{
		if (action_state.render_distance.value >= RENDERER_MIN_RENDER_DISTANCE && action_state.render_distance.value <= RENDERER_MAX_RENDER_DISTANCE) RENDERER.settings.render_distance = action_state.render_distance.value;
	}

	if (action_state.backend_device_index.change);

	if (action_state.backend_index.change);

	if (action_state.vsync.change);

	if (action_state.fps.change);

	if (action_state.max_mipmap_level_count.change);

	/*RENDERER_ACCESS_PUBLIC_STATE(
		renderer_write_settings_to_settings_state(&RENDERER.public.state.settings, &RENDERER.settings);
		renderer_write_backend_settings_to_settings_state(&RENDERER.public.state.settings, &RENDERER.backend.settings);
		RENDERER.public.state.settings_changed = true;

		if (RENDERER.backend.device_infos_changed)
		{
			RENDERER.public.state.info.backend_device_infos = RENDERER.backend.device_infos;
			RENDERER.public.state.info_changed = true;
		}
	);*/

	return MINEC_CLIENT_SUCCESS;
}

uint32_t renderer_frame(struct minec_client* client)
{
	if (renderer_frame_process_actions(client) != MINEC_CLIENT_SUCCESS) return MINEC_CLIENT_ERROR;

	cwindow_freeze_event_queue(APPLICATION_WINDOW.window);

	uint32_t width, height, x, y;
	cwindow_get_dimensions(APPLICATION_WINDOW.window, &width, &height, &x, &y);

	cwindow_unfreeze_event_queue(APPLICATION_WINDOW.window);

	if (RENDERER.frame_info.width != width )

	if (renderer_backend_frame_start(client) != MINEC_CLIENT_SUCCESS)
	{

	}

	renderer_backend_frame_submit(client);

	return MINEC_CLIENT_SUCCESS;
}