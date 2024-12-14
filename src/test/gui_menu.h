#include "game_logging.h"

#include "string.h"

#include "general/logging.h"

uint32_t game_logger_init(struct game_logger* logger, uint8_t* chat_log_folder_path, uint8_t* debug_log_folder_path) {

	time_t raw_time = time(NULL);
	struct tm* time_info = localtime(&raw_time);

	uint8_t file_name[] = {
		 (time_info->tm_mon + 1) / 10 + '0',
		 (time_info->tm_mon + 1) % 10 + '0',
		  time_info->tm_mday / 10 + '0',
		  time_info->tm_mday % 10 + '0',
		 (time_info->tm_year + 1900) / 1000 + '0',
		((time_info->tm_year + 1900) / 100) % 10 + '0',
		((time_info->tm_year + 1900) / 10) % 10 + '0',
		 (time_info->tm_year + 1900) % 10 + '0',
		  time_info->tm_hour / 10 + '0',
		  time_info->tm_hour % 10 + '0',
		  time_info->tm_min / 10 + '0',
		  time_info->tm_min % 10 + '0',
		  time_info->tm_sec / 10 + '0',
		  time_info->tm_sec % 10 + '0',
		  '.',
		  'l',
		  'o',
		  'g'
	};

	uint8_t* path = chat_log_folder_path;

	for (uint32_t i = 0; i < 2; i++) {

		uint32_t path_length = strlen(path);

		uint32_t slash = (path[path_length - 1] == '/');

		uint8_t* file_path = alloca(path_length + strlen(file_name) + slash);

		memcpy(file_path, path, path_length);
		if (!slash) file_path[path_length] = '/';
		memcpy(file_path + path_length + slash, file_name, strlen(file_name));

		if (path == chat_log_folder_path) {
			logger->chat_log_file = fopen(file_path, "w");
			if (logger->chat_log_file == 0) return 1;
		}
		else {
			logger->debug_log_file = fopen(file_path, "w");
			if (logger->debug_log_file == 0) {
				fclose(logger->chat_log_file);
				return 1;
			}
		}

		path = debug_log_folder_path;
	}

	return 0;
}

uint32_t game_logger_exit(struct game_logger* logger) {
	fclose(logger->chat_log_file);
	fclose(logger->debug_log_file);

	return 1;
}
