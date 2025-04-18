#include "application_window.h"

uint32_t application_window_create(struct application_window* window, uint32_t posx, uint32_t posy, uint32_t width, uint32_t height, uint8_t* name)
{
	window->window_handle = window_create(posx, posy, width, height, name, 1);

	if (window->window_handle == WINDOW_CREATION_FAILED) return 1;

	window->width = window_get_width(window->window_handle);
	window->height = window_get_height(window->window_handle);

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

	for (uint32_t i = 0; i < KEY_TOTAL_COUNT; i++) window->input.keyboard[i] &= ~KEY_CHANGE_MASK;

	struct window_event event;
	while (window_process_next_event(window->window_handle, &event)) {

		switch (event.type) {

		case WINDOW_EVENT_DESTROY: {
			return 1;
		} break;

		case WINDOW_EVENT_MOVE_SIZE: {

			window->width = event.info.event_move_size.width;
			window->height = event.info.event_move_size.height;

			if (window->width != 0 && window->height != 0) {

				if (
					window->last_render_width != event.info.event_move_size.width ||
					window->last_render_height != event.info.event_move_size.height
					) window->frame_flags |= FRAME_FLAG_RESIZE;

				window->last_render_width = event.info.event_move_size.width;
				window->last_render_height = event.info.event_move_size.height;
			}

		} break;

		case WINDOW_EVENT_CHAR: {
			if (window->input.character_count < MAX_FRAME_CHAR_INPUTS) {
				window->input.characters[window->input.character_count] = event.info.event_char.unicode;
			}
			window->input.character_count++;
		} break;

		case WINDOW_EVENT_KEY_DOWN: {
			if (!(window->input.keyboard[event.info.event_key_down.key] & KEY_DOWN_MASK))
				window->input.keyboard[event.info.event_key_down.key] = KEY_CHANGE_MASK | KEY_DOWN_MASK;
		} break;

		case WINDOW_EVENT_KEY_UP: {
			window->input.keyboard[event.info.event_key_up.key] = KEY_CHANGE_MASK;
		} break;

		case WINDOW_EVENT_MOUSE_SCROLL: {
			window->input.mouse_scroll_steps = event.info.event_mouse_scroll.scroll_steps;
		} break;

		default:
			break;
		}

	}

	if (window->width != 0 && window->height != 0) window->frame_flags |= FRAME_FLAG_RENDERABLE;

	struct point2d_int mouse_position = window_get_mouse_cursor_position(window->window_handle);

	window->input.mouse_x = mouse_position.x;
	window->input.mouse_y = mouse_position.y;

	return 0;
}

uint32_t application_window_destroy(struct application_window* window)
{
	window_destroy(window->window_handle);

	return 0;
}