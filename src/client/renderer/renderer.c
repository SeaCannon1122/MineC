#include "renderer.h"

#include "client/game_client.h"


uint32_t renderer_create(struct game_client* game) {

	renderer_backend_create(game);

	renderer_backend_load_resources(game);

	return 0;
}

uint32_t renderer_destroy(struct game_client* game) {

	renderer_backend_unload_resources(game);

	renderer_backend_destroy(game);

	return 0;
}

uint32_t renderer_render(struct game_client* game) {

	if ((game->application_state.frame_flags & FRAME_FLAG_SHOULD_RENDER) == 0) return 0;

	if (game->application_state.frame_flags & FRAME_FLAG_SIZE_CHANGE) {
		renderer_backend_resize(game);

	}

	char pixel_str[] = "WWLLOW!";

#define string_to_pixel_char(name, str, size, x, y, flags, r, g, b, a, r_b, g_b, b_b, a_b) struct pixel_render_char name[sizeof(str) - 1];\
for(int i = 0; i < sizeof(str) - 1; i++) {\
if(i == 0) name[i] = (struct pixel_render_char){ { r, g, b, a }, { r_b, g_b, b_b, a_b }, str[i], {x, y}, flags, size };\
else name[i] = (struct pixel_render_char){ { r, g, b, a }, { r_b, g_b, b_b, a_b }, str[i], {name[i-1].position[0] + (size * ((game->resource_state.pixelfont_atlas[PIXELFONT_DEFAULT]->char_font_entries[name[i-1].value].width + 3) / 2 )), y}, flags, size  };\
}\

	string_to_pixel_char(chars, pixel_str, 3, 100 + 3 * game->resource_state.image_atlas[IMAGE_BUTTON].width / 2 - 40, 100 + 3 * game->resource_state.image_atlas[IMAGE_BUTTON].height / 2 - 10, PIXEL_CHAR_UNDERLINE_MASK | PIXEL_CHAR_SHADOW_MASK, 255, 255, 255, 255, 127, 0, 255, 255)

	chars[1].masks |= 2;

	renderer_backend_set_pixel_chars(game, chars, sizeof(pixel_str) - 1);

	struct renderer_rectangle rectangles[8];

	rectangles[0].image_index = IMAGE_DIRT;
	rectangles[0].sampler_index = SAMPLER_DEFAULT;

	rectangles[0].u[0] = 0;
	rectangles[0].u[1] = 0;
	rectangles[0].u[2] = 1;
	rectangles[0].u[3] = 1;

	rectangles[0].v[0] = 0;
	rectangles[0].v[1] = 1;
	rectangles[0].v[2] = 1;
	rectangles[0].v[3] = 0;

	rectangles[0].x[0] = 100;
	rectangles[0].x[1] = 100;
	rectangles[0].x[2] = 100 + 4 * game->resource_state.image_atlas[IMAGE_DIRT].width / 2;
	rectangles[0].x[3] = 100 + 4 * game->resource_state.image_atlas[IMAGE_DIRT].width / 2;

	rectangles[0].y[0] = 100;
	rectangles[0].y[1] = 100 + 4 * game->resource_state.image_atlas[IMAGE_DIRT].height / 2;
	rectangles[0].y[2] = 100 + 4 * game->resource_state.image_atlas[IMAGE_DIRT].height / 2;
	rectangles[0].y[3] = 100;


	renderer_backend_set_rectangles(game, rectangles, 1);

	renderer_backend_render(game);

	return 0;
}

uint32_t renderer_update_resources(struct game_client* game) {

	renderer_backend_unload_resources(game);

	renderer_backend_load_resources(game);
	
	return 0;
}

uint32_t renderer_use_gpu(struct game_client* game, uint32_t gpu_index) {

	renderer_backend_use_gpu(game, gpu_index);

	return 0;
}