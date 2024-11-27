#include "game_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "general/platformlib/platform/platform.h"
#include "general/platformlib/networking.h"

#include "general/utils.h"
#include "general/key_value.h"

#include "client/rendering/gui/pixel_char.h"
#include "client/game_client_body/resources.h"
#include "game_menus.h"
#include "game_client_networker.h"
#include "game_client_renderer.h"
#include "game_client_simulator.h"
#include "game_logging.h"
#include "game/networking_packets/networking_packets.h"

#include "debug.h"


int32_t run_game_client(struct game_client* game, uint8_t* resource_path) {

	init_game_menus(game);
	init_networker(game);
	debug_init(game);

	game->running = true;

	game->window = window_create(200, 100, 1100, 700, "client");

	//void* networking_thread = create_thread((void(*)(void*))networker_thread_function, game);
	//void* simulation_thread = create_thread((void(*)(void*))game_client_simulator_thread_function, game);

	while (get_key_state(KEY_MOUSE_LEFT) & 0b1) sleep_for_ms(10);


	renderer_init(game);

	int32_t render = 1;
	
	while (window_is_active(game->window)) {

		double rendering_start_time = get_time();

		struct window_event event;
		while (window_process_next_event(&event)) if (event.type == WINDOW_EVENT_DESTROY) break;

		uint32_t new_width = window_get_width(window);
		uint32_t new_height = window_get_height(window);

		if (new_width != 0 && new_height != 0) {

			if (screen_size.width != new_width || screen_size.height != new_height) {

				screen_size.width = new_width;
				screen_size.height = new_height;

				//recreate swapchain

			}

			render = 1;
		}
		else render = 0;

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


	window_destroy(game->window);
	game->running = false;
	game->in_game_flag = false;
	game->networker.close_connection_flag = true;

	renderer_exit(game);

	//join_thread(simulation_thread);
	//join_thread(networking_thread);


	return;
}