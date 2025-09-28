#include <minec_client.h>

uint32_t _backend_device_components_create(struct minec_client* client, struct renderer_settings* requested_settings)
{
	uint32_t result = MINEC_CLIENT_SUCCESS;

	bool
		pixelfont_created = false,
		gui_text_created = false,
		menu_gui_created = false
	;

	if (result == MINEC_CLIENT_SUCCESS)
	{
		RENDERER.settings.vsync = requested_settings->vsync;
		RENDERER.settings.triple_buffering = requested_settings->triple_buffering;

		if (RENDERER.backend_device_infos.infos[RENDERER.settings.backend_device_index].disable_vsync_support == false) RENDERER.settings.vsync = true;
		if (RENDERER.backend_device_infos.infos[RENDERER.settings.backend_device_index].triple_buffering_support == false) RENDERER.settings.triple_buffering = false;

		if ((result = renderer_backend_swapchain_create(client, RENDERER.frame_info.width, RENDERER.frame_info.height, RENDERER.settings.vsync, RENDERER.settings.triple_buffering)) == MINEC_CLIENT_SUCCESS)
			RENDERER.components.backend_create_flags.swapchain = true;
	}
	if (result == MINEC_CLIENT_SUCCESS) result = renderer_backend_component_pixelfont_create(client);
	if (result == MINEC_CLIENT_SUCCESS) result = renderer_backend_component_gui_text_create(client);
	if (result == MINEC_CLIENT_SUCCESS) result = renderer_backend_component_gui_text_create(client);

	if (result != MINEC_CLIENT_SUCCESS)
	{
		if (RENDERER.components.backend_create_flags.swapchain) renderer_backend_swapchain_destroy(client);
	}

	return result;
}

void _backend_device_components_destroy(struct minec_client* client)
{
	if (RENDERER.components.backend_create_flags.swapchain) renderer_backend_swapchain_destroy(client);
	RENDERER.components.backend_create_flags.swapchain = false;
}

uint32_t _backend_base_components_create(struct minec_client* client, struct renderer_settings* requested_settings)
{
	RENDERER.settings.backend_device_index = requested_settings->backend_device_index;

	uint32_t result;

	for (int32_t i = -1, original_i = RENDERER.settings.backend_device_index; i < (int32_t)RENDERER.backend_device_infos.count; i++, RENDERER.settings.backend_device_index = i)
	{
		if (i == original_i) continue;
		if (RENDERER.backend_device_infos.infos[RENDERER.settings.backend_device_index].usable == false) continue;
		if ((result = renderer_backend_device_create(client, RENDERER.settings.backend_device_index)) == MINEC_CLIENT_SUCCESS)
		{
			if ((result = _backend_device_components_create(client, requested_settings)) == MINEC_CLIENT_SUCCESS)
			{
				RENDERER.components.backend_create_flags.device = true;
				return MINEC_CLIENT_SUCCESS;
			}
			else
			{
				renderer_backend_device_destroy(client);
				if (result == MINEC_CLIENT_ERROR_BACKEND_CRASHED) return MINEC_CLIENT_ERROR;
			}
		}
		else if (result == MINEC_CLIENT_ERROR_BACKEND_CRASHED) return MINEC_CLIENT_ERROR;
	}
	return MINEC_CLIENT_ERROR;
}

void _backend_base_components_destroy(struct minec_client* client)
{
	_backend_device_components_destroy(client);

	if (RENDERER.components.backend_create_flags.device) renderer_backend_device_destroy(client);
	RENDERER.components.backend_create_flags.device = false;
}

uint32_t _backend_components_create(struct minec_client* client, struct renderer_settings* requested_settings)
{
	RENDERER.settings.backend_index = requested_settings->backend_index;

	for (int32_t i = -1, original_i = RENDERER.settings.backend_index; i < (int32_t)RENDERER.backend_count; i++, RENDERER.settings.backend_index = i)
	{
		if (i == original_i) continue;
		if (renderer_backend_base_create(client, RENDERER.settings.backend_index, APPLICATION_WINDOW.context, APPLICATION_WINDOW.window, &RENDERER.backend_device_infos) == MINEC_CLIENT_SUCCESS)
		{
			if (_backend_base_components_create(client, requested_settings) == MINEC_CLIENT_SUCCESS)
			{
				RENDERER.components.backend_create_flags.base = true;
				RENDERER.backend_device_infos_changed = true;
				return MINEC_CLIENT_SUCCESS;
			}
			renderer_backend_base_destroy(client);
		}
	}
	return MINEC_CLIENT_ERROR;
}

void _backend_components_destroy(struct minec_client* client)
{
	_backend_base_components_destroy(client);

	if (RENDERER.components.backend_create_flags.base) renderer_backend_base_destroy(client);
	RENDERER.components.backend_create_flags.base = false;
}

uint32_t renderer_components_create(struct minec_client* client)
{
	uint32_t result = MINEC_CLIENT_SUCCESS;

	bool
		pixelfont_created = false,
		gui_text_created = false,
		menu_gui_created = false
	;

	int32_t x, y;
	cwindow_get_dimensions(APPLICATION_WINDOW.window, &RENDERER.frame_info.width, &RENDERER.frame_info.height, &x, &y);

	if (result == MINEC_CLIENT_SUCCESS) if ((result = renderer_component_pixelfont_create(client)) == MINEC_CLIENT_SUCCESS) pixelfont_created = true;
	if (result == MINEC_CLIENT_SUCCESS) if ((result = renderer_component_gui_text_create(client)) == MINEC_CLIENT_SUCCESS) gui_text_created = true;
	if (result == MINEC_CLIENT_SUCCESS) if ((result = renderer_component_menu_gui_create(client)) == MINEC_CLIENT_SUCCESS) menu_gui_created = true;

	if (result == MINEC_CLIENT_SUCCESS)
	{
		RENDERER.backend_device_infos_changed = false;
		RENDERER.components.backend_recreate = false;

		renderer_backend_get_infos(client, &RENDERER.public.state.info.backend_infos);
		RENDERER.backend_count = RENDERER.public.state.info.backend_infos.count;

		memset(&RENDERER.components.backend_create_flags, 0, sizeof(RENDERER.components.backend_create_flags));
		struct renderer_settings requested_settings = RENDERER.settings;
		if (_backend_components_create(client, &requested_settings) != MINEC_CLIENT_SUCCESS) result = MINEC_CLIENT_ERROR;
	}
		
	if (result != MINEC_CLIENT_SUCCESS)
	{
		if (menu_gui_created) renderer_component_menu_gui_destroy(client);
		if (gui_text_created) renderer_component_gui_text_destroy(client);
		if (pixelfont_created) renderer_component_pixelfont_destroy(client);
	}

	return result;
}

void renderer_components_destroy(struct minec_client* client)
{
	_backend_components_destroy(client);

	renderer_component_menu_gui_destroy(client);
	renderer_component_gui_text_destroy(client);
	renderer_component_pixelfont_destroy(client);
}

uint32_t renderer_reload_assets(struct minec_client* client)
{
	uint32_t result = MINEC_CLIENT_SUCCESS;

	if (result == MINEC_CLIENT_SUCCESS) renderer_component_menu_gui_reload_assets(client);
	if (result == MINEC_CLIENT_SUCCESS) renderer_component_gui_text_reload_assets(client);
	if (result == MINEC_CLIENT_SUCCESS) renderer_component_menu_gui_reload_assets(client);

	return result;
}

uint32_t renderer_frame_process_actions(struct minec_client* client)
{
	struct renderer_action_state action_state;

	RENDERER_ACCESS_ACTION_STATE(
		action_state = RENDERER.public.action;
		renderer_reset_action_state(client);
	);

	if (action_state.backend_index.set)
	{
		struct renderer_settings settings = RENDERER.settings;
		settings.backend_index = action_state.backend_index.value;

		_backend_components_destroy(client);
		if (_backend_components_create(client, &settings) != MINEC_CLIENT_SUCCESS) return MINEC_CLIENT_ERROR;
	}

	if (action_state.backend_device_index.set)
	{
		struct renderer_settings settings = RENDERER.settings;
		settings.backend_device_index = action_state.backend_device_index.value;

		_backend_base_components_destroy(client);
		if (_backend_base_components_create(client, &settings) != MINEC_CLIENT_SUCCESS)
		{
			RENDERER.components.backend_recreate = true;
			return MINEC_CLIENT_ERROR;
		}
	}

	if ((action_state.vsync.set && RENDERER.backend_device_infos.infos[RENDERER.settings.backend_device_index].disable_vsync_support) || (action_state.triple_buffering.set && RENDERER.backend_device_infos.infos[RENDERER.settings.backend_device_index].triple_buffering_support))
	{
		if (action_state.vsync.set) RENDERER.settings.vsync = action_state.vsync.value;
		if (action_state.triple_buffering.set) RENDERER.settings.triple_buffering = action_state.triple_buffering.value;

		renderer_backend_swapchain_destroy(client);
		if (renderer_backend_swapchain_create(client, RENDERER.frame_info.width, RENDERER.frame_info.height, RENDERER.settings.vsync, RENDERER.settings.triple_buffering) != MINEC_CLIENT_SUCCESS)
		{
			RENDERER.components.backend_create_flags.swapchain = false;
			RENDERER.components.backend_recreate = true;
			return MINEC_CLIENT_SUCCESS;
		}
	}

	if (action_state.fov.set)
	{
		if (action_state.fov.value >= RENDERER_MIN_FOV && action_state.fov.value <= RENDERER_MAX_FOV) RENDERER.settings.fov = action_state.fov.value;
	}
	if (action_state.render_distance.set)
	{
		if (action_state.render_distance.value >= RENDERER_MIN_RENDER_DISTANCE && action_state.render_distance.value <= RENDERER_MAX_RENDER_DISTANCE) RENDERER.settings.render_distance = action_state.render_distance.value;
	}

	if (action_state.fps.set) RENDERER.settings.fps = action_state.fps.value;

	if (action_state.max_mipmap_level_count.set);


	RENDERER_ACCESS_PUBLIC_STATE(
		RENDERER.public.state.settings = RENDERER.settings;
		RENDERER.public.state.settings_changed = true;

		if (RENDERER.backend_device_infos_changed)
		{
			RENDERER.public.state.info.backend_device_infos = RENDERER.backend_device_infos;
			RENDERER.public.state.info_changed = true;
		}
	);

	return MINEC_CLIENT_SUCCESS;
}

uint32_t renderer_frame(struct minec_client* client)
{
	cwindow_freeze_event_queue(APPLICATION_WINDOW.window);

	uint32_t width, height, x, y;
	cwindow_get_dimensions(APPLICATION_WINDOW.window, &width, &height, &x, &y);

	if (width > 0 && height > 0)
	{
		if (width != RENDERER.frame_info.width || height != RENDERER.frame_info.height)
		{
			RENDERER.frame_info.width = width;
			RENDERER.frame_info.height = height;

			if (RENDERER.components.backend_recreate == false)
			{
				renderer_backend_swapchain_destroy(client);
				if (renderer_backend_swapchain_create(client, width, height, RENDERER.settings.vsync, RENDERER.settings.triple_buffering) != MINEC_CLIENT_SUCCESS)
				{
					RENDERER.components.backend_create_flags.swapchain = false;
					RENDERER.components.backend_recreate = true;
				}
			}
		}
	}
	else
	{
		cwindow_unfreeze_event_queue(APPLICATION_WINDOW.window);
		time_sleep(50);
		return MINEC_CLIENT_SUCCESS;
	}

	if (RENDERER.components.backend_recreate)
	{
		RENDERER.components.backend_recreate = false;

		RENDERER_ACCESS_ACTION_STATE(
			if (RENDERER.public.action.backend_index.set == false)
			{
				RENDERER.public.action.backend_index.value = RENDERER.settings.backend_index;
				RENDERER.public.action.backend_index.set = true;
			}
		);
	}

	if (renderer_frame_process_actions(client) != MINEC_CLIENT_SUCCESS)
	{
		cwindow_unfreeze_event_queue(APPLICATION_WINDOW.window);
		if (RENDERER.components.backend_recreate) return MINEC_CLIENT_SUCCESS;
		else return MINEC_CLIENT_ERROR;
	}

	cwindow_unfreeze_event_queue(APPLICATION_WINDOW.window);

	uint32_t result = MINEC_CLIENT_SUCCESS;

	if (result == MINEC_CLIENT_SUCCESS) result = renderer_backend_frame_start(client);

	if (result == MINEC_CLIENT_SUCCESS) result = renderer_component_menu_gui_frame(client);

	if (result == MINEC_CLIENT_SUCCESS) result = renderer_backend_frame_submit(client);

	if (result == MINEC_CLIENT_ERROR_BACKEND_CRASHED)
	{
		RENDERER.components.backend_recreate = true;
		return MINEC_CLIENT_SUCCESS;
	}
	else
	{
		if (RENDERER.settings.fps != 0)
		{
			uint32_t sleep_time = 1000. / (double)RENDERER.settings.fps - (time_get() - RENDERER.frame_info.time);
			if (sleep_time > 0) time_sleep(sleep_time);
		}

		double time_new = time_get();
		RENDERER.frame_info.frame_time = time_new - RENDERER.frame_info.time;
		RENDERER.frame_info.time = time_new;
	}

	return MINEC_CLIENT_SUCCESS;
}