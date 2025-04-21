#include "application_window.h"

#include <string.h>

uint32_t application_window_create(struct application_window* window, uint32_t posx, uint32_t posy, uint32_t width, uint32_t height, uint8_t* name)
{
	window->window_handle = window_create(posx, posy, width, height, name, true, NULL);

	uint32_t position_x, position_y;
	window_get_dimensions(window->window_handle, &window->width, &window->height, &position_x, &position_y);

	window->last_render_width = window->width;
	window->last_render_height = window->height;

	memset(&window->input, 0, sizeof(window->input));

	return 0;
}

uint32_t application_window_handle_events(struct application_window* window)
{
	window->frame_flags = 0;
	window->input.character_count = 0;
	window->input.mouse_scroll_steps = 0;

	for (uint32_t i = 0; i < WINDOW_KEY_TOTAL_COUNT; i++) window->input.keyboard[i] &= ~KEY_CHANGE_MASK;

	struct window_event* event;
	while (event = window_next_event(window->window_handle)) {

		switch (event->type) {

		case WINDOW_EVENT_DESTROY: {
			return 1;
		} break;

		case WINDOW_EVENT_MOVE_SIZE: {

			window->width = event->info.move_size.width;
			window->height = event->info.move_size.height;

			if (window->width != 0 && window->height != 0) {

				if (
					window->last_render_width != event->info.move_size.width ||
					window->last_render_height != event->info.move_size.height
					) window->frame_flags |= FRAME_FLAG_RESIZE;

				window->last_render_width = event->info.move_size.width;
				window->last_render_height = event->info.move_size.height;
			}

		} break;

		case WINDOW_EVENT_CHARACTER: {
			if (window->input.character_count < MAX_FRAME_CHAR_INPUTS) {
				window->input.characters[window->input.character_count] = event->info.character.code_point;
			}
			window->input.character_count++;
		} break;

		case WINDOW_EVENT_KEY_DOWN: {
			if (!(window->input.keyboard[event->info.key_down.key] & KEY_DOWN_MASK))
				window->input.keyboard[event->info.key_down.key] = KEY_CHANGE_MASK | KEY_DOWN_MASK;
		} break;

		case WINDOW_EVENT_KEY_UP: {
			window->input.keyboard[event->info.key_up.key] = KEY_CHANGE_MASK;
		} break;

		case WINDOW_EVENT_MOUSE_SCROLL: {
			window->input.mouse_scroll_steps = event->info.mouse_scroll.scroll_steps;
		} break;

		default:
			break;
		}

	}

	if (window->width != 0 && window->height != 0) window->frame_flags |= FRAME_FLAG_RENDERABLE;

	window_get_mouse_cursor_position(window->window_handle, &window->input.mouse_x, &window->input.mouse_y);

	return 0;
}

uint32_t application_window_destroy(struct application_window* window)
{
	window_destroy(window->window_handle);

	return 0;
}