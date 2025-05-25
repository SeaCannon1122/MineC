#include <minec_client.h>
#include "backend/backend.h"

void rendering_thread_function(struct minec_client* client)
{
	renderer_log(client, "Entered rendering loop");

	bool should_close_bool = false;
	while (true)
	{
		mutex_lock(&client->renderer.thread_mutex);

		if (client->renderer.backend.pipelines_resources.created == false) break;
		
		client->renderer.backend.global.interfaces[client->renderer.backend.global.backend_index].render(client);

		mutex_unlock(&client->renderer.thread_mutex);

		atomic_load_(bool, &client->renderer.thread_should_close, &should_close_bool);
		if (should_close_bool) break;
	}

	renderer_log(client, "Left rendering loop");
}