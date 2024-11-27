#include "game_logging.h"

#include "general/logging.h"

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