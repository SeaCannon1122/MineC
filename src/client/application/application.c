#include "client/game_client.h"

uint32_t application_create(struct game_client* game) {

	uint32_t screen_width = get_screen_width();
	uint32_t screen_height = get_screen_height();

	game->application_state.window = window_create(
		screen_width / 4,
		screen_height / 8,
		screen_width / 2,
		screen_height / 2,
		"Minecraft Clone",
		1
	);

	if (game->application_state.window == WINDOW_CREATION_FAILED) return 1;

	game->application_state.window_extent.width = window_get_width(game->application_state.window);
	game->application_state.window_extent.height = window_get_height(game->application_state.window);

	memset(&game->application_state.input_state, 0, sizeof(game->application_state.input_state));

	return 0;
}

uint32_t application_handle_events(struct game_client* game) {
	
	game->application_state.frame_flags = 0;
	game->application_state.input_state.character_count = 0;

	for (uint32_t i = 0; i < KEY_TOTAL_COUNT; i++) game->application_state.input_state.keyboard[i] &= ~KEY_CHANG_MASK;

	struct window_event event;
	while (window_process_next_event(game->application_state.window, &event)) {

		switch (event.type) {

		case WINDOW_EVENT_DESTROY: {
			return 1;
		} break;

		case WINDOW_EVENT_MOVE_SIZE: {

			game->application_state.window_extent.width = event.info.event_move_size.width;
			game->application_state.window_extent.height = event.info.event_move_size.height;

			if (game->application_state.window_extent.width != 0 && game->application_state.window_extent.height != 0) {

				if (
					game->application_state.last_render_window_extent.width != event.info.event_move_size.width ||
					game->application_state.last_render_window_extent.height != event.info.event_move_size.height
				) game->application_state.frame_flags |= FRAME_FLAG_SIZE_CHANGE;

				game->application_state.last_render_window_extent.width = event.info.event_move_size.width;
				game->application_state.last_render_window_extent.height = event.info.event_move_size.height;
			}

		} break;

		case WINDOW_EVENT_CHAR: {
			if (game->application_state.input_state.character_count < MAX_FRAME_CHAR_INPUTS) {
				game->application_state.input_state.characters[game->application_state.input_state.character_count] = event.info.event_char.unicode;
			}
			game->application_state.input_state.character_count++;
		} break;

		case WINDOW_EVENT_KEY_DOWN: {
			game->application_state.input_state.keyboard[event.info.event_key_down.key] = KEY_CHANG_MASK | KEY_DOWN_MASK;
		} break;

		case WINDOW_EVENT_KEY_UP: {
			game->application_state.input_state.keyboard[event.info.event_key_up.key] = KEY_CHANG_MASK;
		} break;

		default:
			break;
		}

	}

	if (game->application_state.window_extent.width != 0 && game->application_state.window_extent.height != 0) game->application_state.frame_flags |= FRAME_FLAG_SHOULD_RENDER;

	struct point2d_int mouse_position = window_get_mouse_cursor_position(game->application_state.window);

	game->application_state.input_state.mouse_coords.x = mouse_position.x;
	game->application_state.input_state.mouse_coords.y = mouse_position.y;

	game->application_state.time = get_time();

	return 0;
}

uint32_t application_destroy(struct game_client* game) {

	window_destroy(game->application_state.window);

	return 0;
}