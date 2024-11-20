#include "game_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "general/platformlib/platform/platform.h"
#include "general/platformlib/networking.h"

#include "general/utils.h"
#include "general/keyvalue.h"
#include "general/resource_loader.h"
#include "general/resource_manager.h"

#include "client/rendering/gui/pixel_char.h"
#include "client/rendering/gui/menu.h"
#include "client/game_client_body/resources.h"
#include "game_menus.h"
#include "game_client_networker.h"
#include "game_client_renderer.h"
#include "game_client_simulator.h"
#include "game/networking_packets/networking_packets.h"

#include "debug.h"
#include "general/logging.h"


void log_chat_message(struct game_client* game) {
	int message_index = game->chat_stream.next_index;
	game->chat_stream.next_index = (game->chat_stream.next_index + 1) % game->constants.chat_stream_length;

	//struct char_font* font = game->game_menus.chat_menu.font;

	char message_buffer[MAX_SERVER_MESSAGE_LENGTH + 1];

	game->chat_stream.stream[message_index].time = time(NULL);
	int i = 0;
	for (; game->chat_stream.stream[message_index].message[i].value != '\0'; i++) {
		message_buffer[i] = game->chat_stream.stream[message_index].message[i].value;
	}
	message_buffer[i] = game->chat_stream.stream[message_index].message[i].value;


	struct tm* time_info = localtime(&game->chat_stream.stream[message_index].time);

	char time_buffer[] = {
		digit_to_char((time_info->tm_mon + 1) / 10),
		digit_to_char((time_info->tm_mon + 1) % 10),
		'/',
		digit_to_char(time_info->tm_mday / 10),
		digit_to_char(time_info->tm_mday % 10),
		'/',
		digit_to_char((time_info->tm_year + 1900) / 1000),
		digit_to_char(((time_info->tm_year + 1900) / 100) % 10),
		digit_to_char(((time_info->tm_year + 1900) / 10) % 10),
		digit_to_char((time_info->tm_year + 1900) % 10),
		' ',
		digit_to_char(time_info->tm_hour / 10),
		digit_to_char(time_info->tm_hour % 10),
		':',
		digit_to_char(time_info->tm_min / 10),
		digit_to_char(time_info->tm_min % 10),
		':',
		digit_to_char(time_info->tm_sec / 10),
		digit_to_char(time_info->tm_sec % 10),
		' ',
		'\0',
	};

	printf("%s %s\n", time_buffer, message_buffer);
	fprintf(game->chat_log_file, "%s %s\n", time_buffer, message_buffer);

	
	i = 0;
	/*game->chat_stream.stream[message_index].is_chat_line_break[0] = 0;
	while (1) {

		if (game->chat_stream.stream[message_index].message[i].value == '\0') break;

		int width_used = game->constants.chat_indentation_left + game->constants.chat_indentation_right + font->char_font_entries[game->chat_stream.stream[message_index].message[i].value].width;
		i++;

		while (1) {
			if (game->chat_stream.stream[message_index].message[i].value == '\0') break;
			else if (game->chat_stream.stream[message_index].message[i].value == '\n') {
				game->chat_stream.stream[message_index].is_chat_line_break[i] = 1; i++; break;
			}
			else {
				int new_width = width_used + 1 + font->char_font_entries[game->chat_stream.stream[message_index].message[i].value].width;
				if (new_width > game->constants.chat_width) { game->chat_stream.stream[message_index].is_chat_line_break[i] = 1; break; }
				game->chat_stream.stream[message_index].is_chat_line_break[i] = 0;
				width_used = new_width;
			}
			i++;
		}

		

	}*/

	


}


int client_log_init(struct game_client* game) {
	time_t raw_time = time(NULL);
	struct tm* time_info = localtime(&raw_time);

	char* chat_log_path_rel = get_value_from_key(game->resource_manager, "chat_dump").s;
	if (chat_log_path_rel == NULL) return 1;
	char* debug_log_path_rel = get_value_from_key(game->resource_manager, "debug_dump").s;
	if (debug_log_path_rel == NULL) return 2;

	char chat_log_path[1024];
	int i = 0;
	if (chat_log_path_rel[0] != '/')for (; game->resource_folder_path[i] != '\0'; i++) chat_log_path[i] = game->resource_folder_path[i];
	int j = 0;
	for (; chat_log_path_rel[j] != '\0'; j++) chat_log_path[j + i] = chat_log_path_rel[j];

	chat_log_path[i + j] = digit_to_char((time_info->tm_mon + 1) / 10);
	chat_log_path[i + j + 1] = digit_to_char((time_info->tm_mon + 1) % 10);
	chat_log_path[i + j + 2] = digit_to_char(time_info->tm_mday / 10);
	chat_log_path[i + j + 3] = digit_to_char(time_info->tm_mday % 10);
	chat_log_path[i + j + 4] = digit_to_char((time_info->tm_year + 1900) / 1000);
	chat_log_path[i + j + 5] = digit_to_char(((time_info->tm_year + 1900) / 100) % 10);
	chat_log_path[i + j + 6] = digit_to_char(((time_info->tm_year + 1900) / 10) % 10);
	chat_log_path[i + j + 7] = digit_to_char((time_info->tm_year + 1900) % 10);
	chat_log_path[i + j + 8] = digit_to_char(time_info->tm_hour / 10);
	chat_log_path[i + j + 9] = digit_to_char(time_info->tm_hour % 10);
	chat_log_path[i + j + 10] = digit_to_char(time_info->tm_min / 10);
	chat_log_path[i + j + 11] = digit_to_char(time_info->tm_min % 10);
	chat_log_path[i + j + 12] = digit_to_char(time_info->tm_sec / 10);
	chat_log_path[i + j + 13] = digit_to_char(time_info->tm_sec % 10);
	chat_log_path[i + j + 14] = '.';
	chat_log_path[i + j + 15] = 'l';
	chat_log_path[i + j + 16] = 'o';
	chat_log_path[i + j + 17] = 'g';
	chat_log_path[i + j + 18] = '\0';

	char debug_log_path[1024];
	i = 0;
	if (debug_log_path_rel[0] != '/') for (; game->resource_folder_path[i] != '\0'; i++) debug_log_path[i] = game->resource_folder_path[i];
	j = 0;
	for (; debug_log_path_rel[j] != '\0'; j++) debug_log_path[j + i] = debug_log_path_rel[j];

	debug_log_path[i + j] = digit_to_char((time_info->tm_mon + 1) / 10);
	debug_log_path[i + j + 1] = digit_to_char((time_info->tm_mon + 1) % 10);
	debug_log_path[i + j + 2] = digit_to_char(time_info->tm_mday / 10);
	debug_log_path[i + j + 3] = digit_to_char(time_info->tm_mday % 10);
	debug_log_path[i + j + 4] = digit_to_char((time_info->tm_year + 1900) / 1000);
	debug_log_path[i + j + 5] = digit_to_char(((time_info->tm_year + 1900) / 100) % 10);
	debug_log_path[i + j + 6] = digit_to_char(((time_info->tm_year + 1900) / 10) % 10);
	debug_log_path[i + j + 7] = digit_to_char((time_info->tm_year + 1900) % 10);
	debug_log_path[i + j + 8] = digit_to_char(time_info->tm_hour / 10); 
	debug_log_path[i + j + 9] = digit_to_char(time_info->tm_hour % 10);
	debug_log_path[i + j + 10] = digit_to_char(time_info->tm_min / 10);
	debug_log_path[i + j + 11] = digit_to_char(time_info->tm_min % 10);
	debug_log_path[i + j + 12] = digit_to_char(time_info->tm_sec / 10);
	debug_log_path[i + j + 13] = digit_to_char(time_info->tm_sec % 10);
	debug_log_path[i + j + 14] = '.';
	debug_log_path[i + j + 15] = 'l';
	debug_log_path[i + j + 16] = 'o';
	debug_log_path[i + j + 17] = 'g';
	debug_log_path[i + j + 18] = '\0';

	game->chat_log_file = fopen(chat_log_path, "w");
	game->debug_log_file = fopen(debug_log_path, "w");

	return 0;
}

int32_t run_game_client(struct game_client* game, uint8_t* resource_path) {

	init_game_menus(game);
	init_networker(game);
	debug_init(game);

	game->running = true;

	game->window = window_create(200, 100, 1100, 700, "client");

	log_message(game->debug_log_file, "Created window");

	//void* networking_thread = create_thread((void(*)(void*))networker_thread_function, game);
	//void* simulation_thread = create_thread((void(*)(void*))game_client_simulator_thread_function, game);

	while (get_key_state(KEY_MOUSE_LEFT) & 0b1) sleep_for_ms(10);

	game->render_state.width = (window_get_width(game->window) + game->settings.resolution_scale - 1) / game->settings.resolution_scale;
	game->render_state.height = (window_get_height(game->window) + game->settings.resolution_scale - 1) / game->settings.resolution_scale;
	game->render_state.pixels = malloc(game->render_state.width * game->render_state.height * sizeof(unsigned int));

	renderer_init(game);

	log_message(game->debug_log_file, "Entering main Game Loop");

	
	while (window_is_active(game->window)) {

		double rendering_start_time = get_time();

		game->input_state.left_click = get_key_state(KEY_MOUSE_LEFT);
		game->input_state.right_click = get_key_state(KEY_MOUSE_RIGHT);
		game->input_state.escape = get_key_state(KEY_ESCAPE);
		game->input_state.ctrl_left = get_key_state(KEY_CONTROL_L);

		struct window_event event;

		while (window_process_next_event(&event)) {
			switch (event.type)
			{

			case WINDOW_EVENT_SIZE: {
				game->render_state.width = event.info.window_event_size.width;
				game->render_state.height = event.info.window_event_size.height;

				if (game->render_state.height > 0 && game->render_state.width > 0) client_renderer_adjust_size(game);
			} break;
			
			}
		}

		struct point2d_int mouse_position = window_get_mouse_cursor_position(game->window);

		game->input_state.mouse_x = mouse_position.x;
		game->input_state.mouse_y = mouse_position.y;
		game->input_state.left_click = get_key_state(KEY_MOUSE_LEFT);
		game->input_state.right_click = get_key_state(KEY_MOUSE_RIGHT);
		game->input_state.escape = get_key_state(KEY_ESCAPE);
		game->input_state.ctrl_left = get_key_state(KEY_CONTROL_L);

		if (game->in_game_flag && game->render_state.height > 0 && game->render_state.width > 0) { 

			//vk begin rendering

			//client_render_world(game);

			//game_menus_frame(game);

			//vk end rendering

			//display rendering
		}

		while (get_time() - rendering_start_time < 1000. / 60.);

	}

	free(game->render_state.pixels);

	window_destroy(game->window);
	game->running = false;
	game->in_game_flag = false;
	game->networker.close_connection_flag = true;

	renderer_exit(game);

	//join_thread(simulation_thread);
	//join_thread(networking_thread);


	return;
}