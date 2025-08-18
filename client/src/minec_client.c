#include "minec_client.h"

void minec_client_run(uint8_t* data_files_path)
{
	struct minec_client client_memory;
	struct minec_client* client = &client_memory;

	bool status = MINEC_CLIENT_SUCCESS;

	bool
		data_files_path_allocated  = false,
		settings_created = false,
		asset_loader_created = false,
		application_window_created = false,
		renderer_created = false
	;

	if (status == MINEC_CLIENT_SUCCESS)
	{
		if ((client->data_files_path = malloc_string(data_files_path)) == NULL)
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
		}
	}

	if (status == MINEC_CLIENT_SUCCESS)
	{
		settings_load(client);

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
		if (renderer_create(client, &client->settings.video.renderer) != MINEC_CLIENT_SUCCESS)
		{
			status = MINEC_CLIENT_ERROR;
			minec_client_log_error(client, "[GLOBAL] Failed to create Renderer");
		}
		else
		{
			renderer_created = true;
			minec_client_log_info(client, "[GLOBAL] Renderer created");
		}
	}

	if (status == MINEC_CLIENT_SUCCESS) while (application_window_handle_events(client) == MINEC_CLIENT_SUCCESS)
	{
#ifdef MINEC_CLIENT_DYNAMIC_RENDERER_EXECUTABLE
		if (client->window.input.keyboard[WINDOW_KEY_R] == (KEY_DOWN_MASK | KEY_CHANGE_MASK))
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

	if (application_window_created)
	{
		application_window_destroy(client);
		minec_client_log_info(client, "[GLOBAL] Application window destroyed");
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
		minec_client_log_info(client, "[GLOBAL] Renderer destroyed");
	}

	if (data_files_path_allocated) free(client->data_files_path);
}