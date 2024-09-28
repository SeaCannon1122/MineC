#include "game_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "general/platformlib/platform.h"
#include "general/platformlib/networking.h"

#include "general/utils.h"
#include "general/keyvalue.h"
#include "general/resource_loader.h"
#include "general/resource_manager.h"

#include "client/gui/char_font.h"
#include "client/gui/menu.h"
#include "game_menus.h"
#include "game_client_networker.h"
#include "game_client_renderer.h"
#include "game_client_simulator.h"
#include "game_client_control.h"
#include "game/networking_packets/networking_packets.h"

#include "debug.h"
#include "general/logging.h"


void log_chat_message(struct game_client* game) {
	int message_index = game->chat_stream.next_index;
	game->chat_stream.next_index = (game->chat_stream.next_index + 1) % game->constants.chat_stream_length;

	struct char_font* font = game->game_menus.chat_menu.font;

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
	game->chat_stream.stream[message_index].is_chat_line_break[0] = 0;
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

		

	}

	


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

int new_game_client(struct game_client* game, char* resource_path) {

	game->resource_manager = new_resource_manager(resource_path);
	if (game->resource_manager == NULL) {
		printf("Couldn't find critical resourclayout file at expected path %s\n", resource_path);
		return 1;
	}

	int i = 0;
	for (; resource_path[i] != '\0'; i++);
	for (; resource_path[i] != '/' && i > 0; i--);
	if (resource_path[i] == '/') i++;

	for (int j = 0; j < i; j++) game->resource_folder_path[j] = resource_path[j];
	game->resource_folder_path[i] = '\0';

	if (client_log_init(game) != 0) {
		printf("Couldn't find log directory paths in resource_manager\n");
		destroy_resource_manager(game->resource_manager);
		return 2;
	}

	struct key_value_map* settings_map = get_value_from_key(game->resource_manager, "settings").ptr;
	if (settings_map == NULL) {
		log_message(game->debug_log_file, "Couldn't find critical settings file link 'settings' in resourcelayout");
		destroy_resource_manager(game->resource_manager);
		return 2;
	}

	game->settings.render_distance = get_value_from_key(settings_map, "render_distance").i;
	game->settings.gui_scale = get_value_from_key(settings_map, "gui_scale").i;
	game->settings.resolution_scale = get_value_from_key(settings_map, "resolution_scale").i;
	game->settings.fov = get_value_from_key(settings_map, "fov").i;

	struct key_value_map* constants_map = get_value_from_key(game->resource_manager, "constants").ptr;
	if (constants_map == NULL) {
		log_message(game->debug_log_file, "Couldn't find critical settings file link 'constants' in resourcelayout");
		destroy_resource_manager(game->resource_manager);
		return 2;
	}

	game->constants.render_distance_min = get_value_from_key(constants_map, "render_distance_min").i;
	game->constants.render_distance_max = get_value_from_key(constants_map, "render_distance_max").i;
	game->constants.fov_min = get_value_from_key(constants_map, "fov_min").i;
	game->constants.fov_max = get_value_from_key(constants_map, "fov_max").i;
	game->constants.client_connection_timeout = get_value_from_key(constants_map, "client_connection_timeout").i;
	game->constants.packet_awaiting_timeout = get_value_from_key(constants_map, "packet_awaiting_timeout").i;
	game->constants.chat_width = get_value_from_key(constants_map, "chat_width").i;
	game->constants.max_chat_lines_display = get_value_from_key(constants_map, "max_chat_lines_display").i;
	game->constants.chat_display_duration = get_value_from_key(constants_map, "chat_display_duration").i;
	game->constants.chat_stream_length = get_value_from_key(constants_map, "chat_stream_length").i;
	game->constants.chat_indentation_left = get_value_from_key(constants_map, "chat_indentation_left").i;
	game->constants.chat_indentation_right = get_value_from_key(constants_map, "chat_indentation_right").i;
	game->constants.chat_line_radius = get_value_from_key(constants_map, "chat_line_radius").i;

	game->chat_stream.stream = malloc(sizeof(struct chat_stream_element) * game->constants.chat_stream_length);
	game->chat_stream.next_index = 0;
	for (int i = game->constants.chat_stream_length - game->constants.max_chat_lines_display; i < game->constants.chat_stream_length; i++) {
		game->chat_stream.stream[i].time = 0;
	}


	game->in_game_flag = false;
	game->disconnect_flag = 0;
	init_game_menus(game);
	init_networker(game);
	debug_init(game);

	game->running = false;

	return 0;
}


void run_game_client(struct game_client* game) {
	show_console_window();

	game->running = true;

	game->window = create_window(200, 100, 1100, 700, "client");

	log_message(game->debug_log_file, "Created window");

	void* networking_thread = create_thread((void(*)(void*))networker_thread_function, game);
	void* simulation_thread = create_thread((void(*)(void*))game_client_simulator_thread_function, game);
	void* control_thread = create_thread((void(*)(void*))game_client_control_thread_function, game);

	while (get_key_state(KEY_MOUSE_LEFT) & 0b1) sleep_for_ms(10);

	game->render_state.width = (get_window_width(game->window) + game->settings.resolution_scale - 1) / game->settings.resolution_scale;
	game->render_state.height = (get_window_height(game->window) + game->settings.resolution_scale - 1) / game->settings.resolution_scale;
	game->render_state.pixels = malloc(game->render_state.width * game->render_state.height * sizeof(unsigned int));

	log_message(game->debug_log_file, "Entering main Game Loop");


	while (is_window_active(game->window) && !game->game_menus.main_menu.quit_game_button_state) {
		
		int new_width = (get_window_width(game->window) + game->settings.resolution_scale - 1) / game->settings.resolution_scale;
		int new_height = (get_window_height(game->window) + game->settings.resolution_scale - 1) / game->settings.resolution_scale;
		if (new_width != game->render_state.width || new_height != game->render_state.height) {
			free(game->render_state.pixels);
			game->render_state.pixels = malloc(new_width * new_height * sizeof(unsigned int));
			game->render_state.width = new_width;
			game->render_state.height = new_height;
		}

		if (game->in_game_flag) { 

			

			client_render_world(game); 
		}

		game_menus_frame(game);

		draw_to_window(game->window, game->render_state.pixels, game->render_state.width, game->render_state.height, game->settings.resolution_scale);

		sleep_for_ms(10);
	}

	free(game->render_state.pixels);

	close_window(game->window);
	game->running = false;
	game->in_game_flag = false;

	join_thread(control_thread);
	join_thread(simulation_thread);
	join_thread(networking_thread);


	return;
}



void delete_game_client(struct game_client* game) {
	destroy_resource_manager(game->resource_manager);
	free(game->chat_stream.stream);
}