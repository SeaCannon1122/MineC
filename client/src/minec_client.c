#include "minec_client.h"

void* minec_client_load_file(uint8_t* path, size_t* size) {

	FILE* file = fopen(path, "rb");

	if (file == NULL) return NULL;

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	rewind(file);

	void* buffer = malloc(fileSize);
	if (buffer == NULL) {
		fclose(file);
		return NULL;
	}

	size_t read = fread(buffer, 1, fileSize, file);

	fclose(file);

	if (read != fileSize)
	{
		free(buffer);
		return NULL;
	}

	*size = read;

	return buffer;
}


uint32_t minec_client_run(struct minec_client* client, uint8_t* runtime_files_path)
{
	client->string_allocator = string_allocator_new(4096);
	client->runtime_files_path = string_allocate_string(client->string_allocator, runtime_files_path);
	client->runtime_files_path_length = strlen(runtime_files_path);

	uint32_t ret = application_window_create(
		&client->main_window,
		100,
		100,
		700,
		500,
		"MineC"
	);

	settings_create(client);
	settings_load(client);

	resources_create(client);

	renderer_create(client);

	/*gui_menus_create(client);
	renderer_create(client);

	simulator_start(client);
	networker_start(client);*/

#ifdef UNIX
	while (get_key_state(KEY_MOUSE_LEFT) & 0b1 != 0) sleep_for_ms(1);
#endif // UNIX

	while (application_window_handle_events(&client->main_window) == 0)
	{

		/*gui_menus_simulation_frame(client);

		if (gui_is_button_clicked(client->gui_menus_state.main.menu_handle, client->gui_menus_state.main.quit_game_button)) break;*/

		//renderer_render(client);

		sleep_for_ms(16);
	}

	/*networker_stop(client);
	simulator_stop(client);

	renderer_destroy(client);
	gui_menus_destroy(client);*/

	renderer_destroy(client);

	resources_destroy(client);

	settings_destroy(client);

	application_window_destroy(&client->main_window);
	string_allocator_delete(client->string_allocator);

	return 0;
}