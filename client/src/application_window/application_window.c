#include <minec_client.h>

#include <stb_image/stb_image.h>

uint32_t application_window_create(struct minec_client* client, uint32_t posx, uint32_t posy, uint32_t width, uint32_t height, uint8_t* name)
{
	if (window_init_context(NULL) != 0) return MINEC_CLIENT_ERROR;

	client->window.window_handle = window_create(posx, posy, width, height, name, true);
	if (client->window.window_handle == NULL)
	{
		minec_client_log_error(client, "Error creating window");
		return MINEC_CLIENT_ERROR;
	}

	size_t raw_window_icon_data_size;
	void* raw_window_icon_data = cerialized_get_file(cerialized_assets_file_system, "icon.png", &raw_window_icon_data_size);

	uint32_t icon_width, icon_height, comp;
	uint32_t* icon_data = stbi_load_from_memory(raw_window_icon_data, raw_window_icon_data_size, &icon_width, &icon_height, &comp, 4);

	window_set_icon(client->window.window_handle, icon_data, icon_width, icon_height);

	atomic_init(&client->window.width);
	atomic_init(&client->window.height);

	atomic_store_(uint32_t, &client->window.width, &width);
	atomic_store_(uint32_t, &client->window.height, &height);

	memset(&client->window.input, 0, sizeof(client->window.input));

	return MINEC_CLIENT_SUCCESS;
}

uint32_t application_window_handle_events(struct minec_client* client)
{
	client->window.input.character_count = 0;
	client->window.input.mouse_scroll_steps = 0;

	for (uint32_t i = 0; i < WINDOW_KEY_TOTAL_COUNT; i++) client->window.input.keyboard[i] &= ~KEY_CHANGE_MASK;

	struct window_event* event;
	while (event = window_next_event(client->window.window_handle)) {

		switch (event->type) {

		case WINDOW_EVENT_DESTROY: {
			return 1;
		} break;

		case WINDOW_EVENT_MOVE_SIZE: {

			atomic_store_(uint32_t, &client->window.width, &event->info.move_size.width);
			atomic_store_(uint32_t, &client->window.height, &event->info.move_size.height);

		} break;

		case WINDOW_EVENT_CHARACTER: {
			if (client->window.input.character_count < MAX_FRAME_CHAR_INPUTS) {
				client->window.input.characters[client->window.input.character_count] = event->info.character.code_point;
			}
			client->window.input.character_count++;
		} break;

		case WINDOW_EVENT_KEY_DOWN: {
			if (!(client->window.input.keyboard[event->info.key_down.key] & KEY_DOWN_MASK))
				client->window.input.keyboard[event->info.key_down.key] = KEY_CHANGE_MASK | KEY_DOWN_MASK;
		} break;

		case WINDOW_EVENT_KEY_UP: {
			client->window.input.keyboard[event->info.key_up.key] = KEY_CHANGE_MASK;
		} break;

		case WINDOW_EVENT_MOUSE_SCROLL: {
			client->window.input.mouse_scroll_steps = event->info.mouse_scroll.scroll_steps;
		} break;

		default:
			break;
		}

	}

	window_get_mouse_cursor_position(client->window.window_handle, &client->window.input.mouse_x, &client->window.input.mouse_y);

	return MINEC_CLIENT_SUCCESS;
}

uint32_t application_window_destroy(struct minec_client* client)
{
	atomic_deinit(&client->window.width);
	atomic_deinit(&client->window.height);

	window_destroy(client->window.window_handle);

	window_deinit_context();

	return 0;
}