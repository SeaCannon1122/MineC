#include <minec_client.h>

uint32_t application_window_create(struct minec_client* client)
{
	uint32_t width = 700, height = 500;

	if ((APPLICATION_WINDOW.context = cwindow_context_create("MineC_Client_window_context")) == NULL)
	{
		minec_client_log_debug_l(client, "'cwindow_context_create(\"MineC_Client_window_context\")' failed");
		return MINEC_CLIENT_ERROR;
	}

	if ((APPLICATION_WINDOW.window = cwindow_create(APPLICATION_WINDOW.context, 100, 100, width, height, "MineC", true)) == NULL)
	{
		minec_client_log_error(client, "[WINDOW] Failed to create window");
		minec_client_log_debug_l(client, "'cwindow_create' failed");
		return MINEC_CLIENT_ERROR;
	}

	size_t raw_window_icon_data_size;
	uint8_t* raw_window_icon_data = resource_index_query("window_icon.png", &raw_window_icon_data_size);
	if (raw_window_icon_data != NULL)
	{
		uint32_t icon_width, icon_height, comp;
		uint32_t* icon_data = (uint32_t*)stbi_load_from_memory((const stbi_uc*)raw_window_icon_data, raw_window_icon_data_size, &icon_width, &icon_height, &comp, 4);
		asset_loader_release_asset(client);
		if (icon_data != NULL)
		{
			if (cwindow_set_icon(APPLICATION_WINDOW.window, icon_data, icon_width, icon_height) == false)
			{
				minec_client_log_error(client, "[WINDOW] Failed to set window icon");
				minec_client_log_debug_l(client, "'cwindow_set_icon' failed");
			}
			free(icon_data);
		}
		else
		{
			minec_client_log_error(client, "[WINDOW] Failed to set window icon");
			minec_client_log_debug_l(client, "'stbi_load_from_memory' with icon data failed, THIS SHOULD NOT HAPPEN. Check cerialized resources");
		}
	}
	else minec_client_log_debug_l(client, "'minec_client_get_resource(\"window_icon.png\", &raw_window_icon_data_size)' failed, THIS SHOULD NOT HAPPEN. Check cerialized resources");

	atomic_uint32_t_init(&APPLICATION_WINDOW.width, width);
	atomic_uint32_t_init(&APPLICATION_WINDOW.height, height);

	memset(&APPLICATION_WINDOW.input, 0, sizeof(APPLICATION_WINDOW.input));

	return MINEC_CLIENT_SUCCESS;
}

uint32_t application_window_events(struct minec_client* client)
{
	APPLICATION_WINDOW.input.character_count = 0;
	APPLICATION_WINDOW.input.mouse_scroll_steps = 0;

	for (uint32_t i = 0; i < CWINDOW_KEY_TOTAL_COUNT; i++) APPLICATION_WINDOW.input.keyboard[i] &= ~KEY_CHANGE_MASK;

	struct cwindow_event* event;
	while (event = cwindow_next_event(APPLICATION_WINDOW.window)) {

		switch (event->type) {

		case CWINDOW_EVENT_DESTROY: {
			return MINEC_CLIENT_ERROR;
		} break;

		case CWINDOW_EVENT_MOVE_SIZE: {

			atomic_uint32_t_store(&APPLICATION_WINDOW.width, event->info.move_size.width);
			atomic_uint32_t_store(&APPLICATION_WINDOW.height, event->info.move_size.height);

		} break;

		case CWINDOW_EVENT_CHARACTER: {
			if (APPLICATION_WINDOW.input.character_count < MAX_FRAME_CHAR_INPUTS) {
				APPLICATION_WINDOW.input.characters[APPLICATION_WINDOW.input.character_count] = event->info.character.code_point;
			}
			APPLICATION_WINDOW.input.character_count++;
		} break;

		case CWINDOW_EVENT_KEY_DOWN: {
			if (!(APPLICATION_WINDOW.input.keyboard[event->info.key_down.key] & KEY_DOWN_MASK))
				APPLICATION_WINDOW.input.keyboard[event->info.key_down.key] = KEY_CHANGE_MASK | KEY_DOWN_MASK;
		} break;

		case CWINDOW_EVENT_KEY_UP: {
			APPLICATION_WINDOW.input.keyboard[event->info.key_up.key] = KEY_CHANGE_MASK;
		} break;

		case CWINDOW_EVENT_MOUSE_SCROLL: {
			APPLICATION_WINDOW.input.mouse_scroll_steps = event->info.mouse_scroll.scroll_steps;
		} break;

		default:
			break;
		}

	}

	cwindow_get_mouse_cursor_position(APPLICATION_WINDOW.window, &APPLICATION_WINDOW.input.mouse_x, &APPLICATION_WINDOW.input.mouse_y);

	return MINEC_CLIENT_SUCCESS;
}

uint32_t application_window_destroy(struct minec_client* client)
{
	atomic_uint32_t_deinit(&APPLICATION_WINDOW.width);
	atomic_uint32_t_deinit(&APPLICATION_WINDOW.height);

	cwindow_destroy(APPLICATION_WINDOW.window);

	cwindow_context_destroy(APPLICATION_WINDOW.context);

	return MINEC_CLIENT_SUCCESS;
}