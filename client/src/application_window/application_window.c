#include <minec_client.h>

#include <stb_image/stb_image.h>

uint32_t application_window_create(struct minec_client* client)
{
	uint32_t width = 700, height = 500;

	if (window_init_context(NULL) == false)
	{
		minec_client_log_debug_error(client, "window_init_context(NULL) failed");
		return MINEC_CLIENT_ERROR;
	}

	WINDOW.window_context_handle = window_get_context();

	if ((WINDOW.window_handle = window_create(100, 100, width, height, "MineC", true)) == NULL)
	{
		minec_client_log_error(client, "[WINDOW] Failed to create window");
		minec_client_log_debug_error(client, "window_create failed");
		return MINEC_CLIENT_ERROR;
	}

	size_t raw_window_icon_data_size;
	const void* raw_window_icon_data = cerialized_get_file(cerialized_resources_file_system, "icon.png", &raw_window_icon_data_size);
	if (raw_window_icon_data != NULL)
	{
		uint32_t icon_width, icon_height, comp;
		uint32_t* icon_data = (uint32_t*)stbi_load_from_memory((const stbi_uc*)raw_window_icon_data, raw_window_icon_data_size, &icon_width, &icon_height, &comp, 4);
		if (icon_data != NULL)
		{
			if (window_set_icon(WINDOW.window_handle, icon_data, icon_width, icon_height) == false)
			{
				minec_client_log_error(client, "[WINDOW] Failed to set window icon");
				minec_client_log_debug_error(client, "window_set_icon failed");
			}
			free(icon_data);
		}
		else
		{
			minec_client_log_error(client, "[WINDOW] Failed to set window icon");
			minec_client_log_debug_error(client, "stbi_load_from_memory failed when loading icon data, THIS SHOULD NOT HAPPEN. Check if icon file is actually a png");
		}
	}
	else
	{
		minec_client_log_error(client, "[WINDOW] Failed to set window icon");
		minec_client_log_debug_error(client, "cerialized_get_file(cerialized_resources_file_system, \"icon.png\", &raw_window_icon_data_size) failed, THIS SHOULD NOT HAPPEN. Check completeness of cerialized resources");
	}

	atomic_uint32_t_init(&WINDOW.width, width);
	atomic_uint32_t_init(&WINDOW.height, height);

	memset(&WINDOW.input, 0, sizeof(WINDOW.input));

	return MINEC_CLIENT_SUCCESS;
}

uint32_t application_window_handle_events(struct minec_client* client)
{
	WINDOW.input.character_count = 0;
	WINDOW.input.mouse_scroll_steps = 0;

	for (uint32_t i = 0; i < WINDOW_KEY_TOTAL_COUNT; i++) WINDOW.input.keyboard[i] &= ~KEY_CHANGE_MASK;

	struct window_event* event;
	while (event = window_next_event(WINDOW.window_handle)) {

		switch (event->type) {

		case WINDOW_EVENT_DESTROY: {
			return MINEC_CLIENT_ERROR;
		} break;

		case WINDOW_EVENT_MOVE_SIZE: {

			atomic_uint32_t_store(&WINDOW.width, event->info.move_size.width);
			atomic_uint32_t_store(&WINDOW.height, event->info.move_size.height);

		} break;

		case WINDOW_EVENT_CHARACTER: {
			if (WINDOW.input.character_count < MAX_FRAME_CHAR_INPUTS) {
				WINDOW.input.characters[WINDOW.input.character_count] = event->info.character.code_point;
			}
			WINDOW.input.character_count++;
		} break;

		case WINDOW_EVENT_KEY_DOWN: {
			if (!(WINDOW.input.keyboard[event->info.key_down.key] & KEY_DOWN_MASK))
				WINDOW.input.keyboard[event->info.key_down.key] = KEY_CHANGE_MASK | KEY_DOWN_MASK;
		} break;

		case WINDOW_EVENT_KEY_UP: {
			WINDOW.input.keyboard[event->info.key_up.key] = KEY_CHANGE_MASK;
		} break;

		case WINDOW_EVENT_MOUSE_SCROLL: {
			WINDOW.input.mouse_scroll_steps = event->info.mouse_scroll.scroll_steps;
		} break;

		default:
			break;
		}

	}

	window_get_mouse_cursor_position(WINDOW.window_handle, &WINDOW.input.mouse_x, &WINDOW.input.mouse_y);

	return MINEC_CLIENT_SUCCESS;
}

uint32_t application_window_destroy(struct minec_client* client)
{
	atomic_uint32_t_deinit(&WINDOW.width);
	atomic_uint32_t_deinit(&WINDOW.height);

	window_destroy(WINDOW.window_handle);

	window_deinit_context();

	return MINEC_CLIENT_SUCCESS;
}