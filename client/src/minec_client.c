#include "minec_client.h"

void minec_client_reload_assets(struct minec_client* client)
{
	asset_loader_reload(client);
}

void minec_client_run(uint8_t* data_files_path)
{
	struct minec_client* client = &(struct minec_client) { 0 };

	bool status = MINEC_CLIENT_SUCCESS;

	bool
		data_files_path_allocated = false,
		settings_created = false,
		string_index_created = false,
		asset_loader_created = false,
		application_window_created = false,
		gui_state_created = false,
		renderer_created = false
	;

	if (status == MINEC_CLIENT_SUCCESS)
	{
		if ((client->data_files_path = string8_malloc(data_files_path)) == NULL)
		{
			status = MINEC_CLIENT_ERROR;
			minec_client_log_out_of_memory(client, "[GLOBAL]", "malloc_string(%s)", data_files_path);
		}
		else data_files_path_allocated = true;
	}
	
	if (status == MINEC_CLIENT_SUCCESS)
	{
		client->data_files_path_length = strlen(data_files_path);

		if (settings_create(client) != MINEC_CLIENT_SUCCESS)
		{
			status = MINEC_CLIENT_ERROR;
			minec_client_log_info(client, "[GLOBAL] Failed to create Settings");
		}
		else
		{
			settings_created = true;
			minec_client_log_info(client, "[GLOBAL] Settings created");

			settings_load(client);
		}
	}

	if (status == MINEC_CLIENT_SUCCESS)
	{
		if (asset_loader_create(client) != MINEC_CLIENT_SUCCESS)
		{
			status = MINEC_CLIENT_ERROR;
			minec_client_log_info(client, "[GLOBAL] Failed to create Asset Manager");
		}
		else
		{
			asset_loader_created = true;
			minec_client_log_info(client, "[GLOBAL] Asset Manager created");
		}
	}

	if (status == MINEC_CLIENT_SUCCESS)
	{
		if (string_index_create(client, &client->settings.general.language.infos, &client->settings.general.language.count) != MINEC_CLIENT_SUCCESS)
		{
			status = MINEC_CLIENT_ERROR;
			minec_client_log_info(client, "[GLOBAL] Failed to create String Index");
		}
		else
		{
			string_index_created = true;
			minec_client_log_info(client, "[GLOBAL] String Index created");

			if (client->settings.general.language.index >= client->settings.general.language.count) client->settings.general.language.index = 0;
			string_index_set_language_index(client, client->settings.general.language.index);
		}
	}

	if (status == MINEC_CLIENT_SUCCESS)
	{
		if (application_window_create(client) != MINEC_CLIENT_SUCCESS)
		{
			status = MINEC_CLIENT_ERROR;
			minec_client_log_info(client, "[GLOBAL] Failed to create Application window");
		}
		else
		{
			application_window_created = true;
			minec_client_log_info(client, "[GLOBAL] Application window created");
		}
	}

	if (status == MINEC_CLIENT_SUCCESS)
	{
		if (gui_state_create(client) != MINEC_CLIENT_SUCCESS)
		{
			status = MINEC_CLIENT_ERROR;
			minec_client_log_error(client, "[GLOBAL] Failed to create GUI State");
		}
		else
		{
			gui_state_created = true;
			minec_client_log_info(client, "[GLOBAL] GUI State created");
		}
	}

	if (status == MINEC_CLIENT_SUCCESS)
	{
		if (renderer_create(client, &client->settings.video.renderer) != MINEC_CLIENT_SUCCESS)
		{
			status = MINEC_CLIENT_ERROR;
			minec_client_log_error(client, "[GLOBAL] Failed to create Renderer");
		}
		else
		{
			renderer_created = true;
			minec_client_log_info(client, "[GLOBAL] Renderer created");

			renderer_get_settings_state(client, &client->settings.video.renderer);
			renderer_get_info_state(client, &client->settings.video.renderer_info);
		}
	}

	if (status == MINEC_CLIENT_SUCCESS) while (application_window_events(client) == MINEC_CLIENT_SUCCESS)
	{
#ifdef MINEC_CLIENT_DYNAMIC_RENDERER_EXECUTABLE
		if (client->window.input.keyboard[CWINDOW_KEY_R] == (KEY_DOWN_MASK | KEY_CHANGE_MASK))
		{
			minec_client_log_info(client, "[GLOBAL] Reloading Renderer");

			if (renderer_reload(client) != MINEC_CLIENT_SUCCESS)
			{
				minec_client_log_info(client, "[GLOBAL] Fatal error while attempting to reloading Renderer");
				renderer_created = false;
				break;
			}
		}
#endif

		if (APPLICATION_WINDOW.input.keyboard[CWINDOW_KEY_T] == (KEY_DOWN_MASK | KEY_CHANGE_MASK))
			settings_load(client);
		if (APPLICATION_WINDOW.input.keyboard[CWINDOW_KEY_Z] == (KEY_DOWN_MASK | KEY_CHANGE_MASK))
			minec_client_reload_assets(client);

		if (renderer_did_crash(client))
		{
			minec_client_log_info(client, "[GLOBAL] Renderer crashed");
			break;
		}

		time_sleep(20);
	}

	if (renderer_created)
	{
		renderer_destroy(client);
		minec_client_log_info(client, "[GLOBAL] Renderer destroyed");
	}

	if (gui_state_created)
	{
		gui_state_destroy(client);
		minec_client_log_info(client, "[GLOBAL] GUI State destroyed");
	}

	if (application_window_created)
	{
		application_window_destroy(client);
		minec_client_log_info(client, "[GLOBAL] Application window destroyed");
	}
	
	if (string_index_created)
	{
		string_index_destroy(client);
		minec_client_log_info(client, "[GLOBAL] String Index destroyed");
	}

	if (asset_loader_created)
	{
		asset_loader_destroy(client);
		minec_client_log_info(client, "[GLOBAL] Asset manager destroyed");
	}

	if (settings_created)
	{
		settings_save(client);
		settings_destroy(client);
		minec_client_log_info(client, "[GLOBAL] Settings destroyed");
	}

	if (data_files_path_allocated) free(client->data_files_path);
}