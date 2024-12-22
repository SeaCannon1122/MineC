#include "../renderer_backend.h"

#include "client/game_client.h"

#include "client/game_client.h"
#include "general/rendering/rendering_window.h"
#include <malloc.h>

#include "renderer_device.c"
#include "renderer_instance.c"
#include "renderer_reources.c"


uint32_t renderer_backend_create(struct game_client* game) {

	renderer_backend_instance_create(game);

	renderer_backend_list_gpus(game);

	game->renderer_state.backend.device = 0;
	for (int i = 0; i < game->application_state.machine_info.gpu_count; i++) if (game->application_state.machine_info.gpus[i].usable) {

		game->application_state.machine_info.gpu_index = i;

		renderer_backend_device_create(game, i);
		break;
	}

	if (game->renderer_state.backend.device == 0) {
		renderer_backend_device_destroy(game);
		return 1;
	}

	return 0;
}


uint32_t renderer_backend_destroy(struct game_client* game) {

	if (game->renderer_state.backend.device != 0) renderer_backend_device_destroy(game);

	renderer_backend_instance_destroy(game);

	return 0;
}

uint32_t renderer_backend_use_gpu(struct game_client* game, uint32_t gpu_index) {
	if (gpu_index >= game->application_state.machine_info.gpu_count) return 1;
	if (game->application_state.machine_info.gpus[gpu_index].usable == 0) return;

	renderer_backend_device_destroy(game);
	renderer_backend_device_create(game, gpu_index);

	return 0;
}

uint32_t renderer_backend_resize(struct game_client* game) {

	rendering_window_resize(game->application_state.window);

	return 0;
}