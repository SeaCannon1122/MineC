#include <minec_client.h>
#include "backend/backend.h"

void rendering_thread_function(struct minec_client* client)
{
	minec_client_log(client, "[RENDERER][THREAD] Entered rendering loop");

	mutex_lock(&client->renderer.mutex);
	client->RENDERER.backend.global.interfaces[client->RENDERER.backend.global.backend_index].start_rendering(client);

	uint32_t request_flag;
	while (true)
	{
		atomic_load_(uint32_t, &client->renderer.request_flag, &request_flag);

		if (request_flag == RENDERER_REQUEST_CLOSE) break;
		else if (request_flag == RENDERER_REQUEST_HALT)
		{

			client->RENDERER.backend.global.interfaces[client->RENDERER.backend.global.backend_index].stop_rendering(client);
			mutex_unlock(&client->renderer.mutex);

			request_flag = RENDERER_REQUEST_RENDER;
			atomic_store_(uint32_t, &client->renderer.request_flag, &request_flag);
			sleep_for_ms(1);

			mutex_lock(&client->renderer.mutex);
			client->RENDERER.backend.global.interfaces[client->RENDERER.backend.global.backend_index].start_rendering(client);
		}

		uint32_t width, height;
		atomic_load_(uint32_t, &client->window.width, &width);
		atomic_load_(uint32_t, &client->window.height, &height);

		

		uint8_t str[32];
		Pixelchar c[32];

		sprintf(str, "%d", client->renderer.thread_state.frame_info.index);
		uint32_t str_len = strlen(str);

		uint32_t scale = 2;

		for (uint32_t i = 0; i < str_len; i++)
		{
			c[i].character = str[i];
			c[i].flags = PIXELCHAR_BACKGROUND_BIT | PIXELCHAR_UNDERLINE_BIT | PIXELCHAR_SHADOW_BIT;
			c[i].fontIndex = i % 2;
			c[i].scale = scale;

			c[i].position[1] = 100;

			if (i == 0) c[i].position[0] = 100;
			else c[i].position[0] = c[i - 1].position[0] + pixelcharGetCharacterRenderingWidth(client->renderer.pixelchar_renderer, &c[i - 1]) + pixelcharGetCharacterRenderingSpacing(client->renderer.pixelchar_renderer, &c[i - 1], &c[i]);

			c[i].color[0] = 0xdc;
			c[i].color[1] = 0xdc;
			c[i].color[2] = 0xdc;
			c[i].color[3] = 255;
			c[i].backgroundColor[0] = 255;
			c[i].backgroundColor[1] = 0;
			c[i].backgroundColor[2] = 0;
			c[i].backgroundColor[3] = 255;

		}

		pixelcharRendererResetQueue(client->renderer.pixelchar_renderer);
		pixelcharRendererEnqueCharacters(client->renderer.pixelchar_renderer, c, str_len);

		client->RENDERER.backend.global.interfaces[client->RENDERER.backend.global.backend_index].render(client);

		client->renderer.thread_state.frame_info.index++;
	}

	client->RENDERER.backend.global.interfaces[client->RENDERER.backend.global.backend_index].stop_rendering(client);
	mutex_unlock(&client->renderer.mutex);

	minec_client_log(client, "[RENDERER][THREAD] Left rendering loop");
}