#include "renderer.h"

#include "client/game_client.h"

#include "renderer_images.h"

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

	if (game->application_state.frame_flags & FRAME_FLAG_SHOULD_RENDER == 0) return 0;

	if (game->application_state.frame_flags & FRAME_FLAG_SIZE_CHANGE) {
		renderer_backend_resize(game);

	}

	char pixel_str[] = "WWLLOW!";

#define string_to_pixel_char(name, str, size, x, y, flags, r, g, b, a, r_b, g_b, b_b, a_b) struct pixel_render_char name[sizeof(str) - 1];\
for(int i = 0; i < sizeof(str) - 1; i++) {\
if(i == 0) name[i] = (struct pixel_render_char){ { r, g, b, a }, { r_b, g_b, b_b, a_b }, str[i], {x, y}, flags, size };\
else name[i] = (struct pixel_render_char){ { r, g, b, a }, { r_b, g_b, b_b, a_b }, str[i], {name[i-1].position[0] + (size * ((game->resource_state.pixelfont_atlas[RESOURCE_PIXEL_FONT_DEFAULT]->char_font_entries[name[i-1].value].width + 3) / 2 )), y}, flags, size  };\
}\

	string_to_pixel_char(chars, pixel_str, 3, 100 + 3 * game->resource_state.image_atlas[RESOURCE_IMAGE_BUTTON].width / 2 - 40, 100 + 3 * game->resource_state.image_atlas[RESOURCE_IMAGE_BUTTON].height / 2 - 10, PIXEL_CHAR_UNDERLINE_MASK | PIXEL_CHAR_SHADOW_MASK, 255, 255, 255, 255, 127, 0, 255, 255)

	chars[1].masks |= 2;

	renderer_backend_set_pixel_chars(game, chars, sizeof(pixel_str) - 1);

	struct renderer_rectangle rectangles[8];

	rectangles[0].image_index = RESOURCE_IMAGE_DIRT;
	rectangles[0].sampler_index = SAMPLING_CONFIGURATION_DEFAULT;

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
	rectangles[0].x[2] = 100 + game->resource_state.image_atlas[RESOURCE_IMAGE_DIRT].width / 2;
	rectangles[0].x[3] = 100 + game->resource_state.image_atlas[RESOURCE_IMAGE_DIRT].width / 2;

	rectangles[0].y[0] = 100;
	rectangles[0].y[1] = 100 + game->resource_state.image_atlas[RESOURCE_IMAGE_DIRT].height / 2;
	rectangles[0].y[2] = 100 + game->resource_state.image_atlas[RESOURCE_IMAGE_DIRT].height / 2;
	rectangles[0].y[3] = 100;

	rectangles[1].image_index = RESOURCE_IMAGE_DIRT;
	rectangles[1].sampler_index = SAMPLING_CONFIGURATION_SMOOTH;

	rectangles[1].u[0] = 0;
	rectangles[1].u[1] = 0;
	rectangles[1].u[2] = 1;
	rectangles[1].u[3] = 1;

	rectangles[1].v[0] = 0;
	rectangles[1].v[1] = 1;
	rectangles[1].v[2] = 1;
	rectangles[1].v[3] = 0;

	rectangles[1].x[0] = 100 + game->resource_state.image_atlas[RESOURCE_IMAGE_DIRT].width / 2;
	rectangles[1].x[1] = 100 + game->resource_state.image_atlas[RESOURCE_IMAGE_DIRT].width / 2;
	rectangles[1].x[2] = 100 + game->resource_state.image_atlas[RESOURCE_IMAGE_DIRT].width;
	rectangles[1].x[3] = 100 + game->resource_state.image_atlas[RESOURCE_IMAGE_DIRT].width;

	rectangles[1].y[0] = 100;
	rectangles[1].y[1] = 100 + game->resource_state.image_atlas[RESOURCE_IMAGE_DIRT].height / 2;
	rectangles[1].y[2] = 100 + game->resource_state.image_atlas[RESOURCE_IMAGE_DIRT].height / 2;
	rectangles[1].y[3] = 100;

	renderer_backend_set_rectangles(game, rectangles, 2);

	renderer_backend_render(game);

	return 0;
}

uint32_t renderer_reload_resources(struct game_client* game) {

	renderer_backend_unload_resources(game);

	renderer_backend_load_resources(game);
	
	return 0;
}