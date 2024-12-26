#include "renderer.h"

#include "client/game_client.h"

#include <stdio.h>
#include <string.h>

uint32_t renderer_load_samplers(struct game_client* game);

uint32_t renderer_create(struct game_client* game) {

	renderer_load_samplers(game);

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

#define string_to_pixel_char(name, str, size, x, y, flags, r, g, b, a, r_b, g_b, b_b, a_b) struct pixel_char name[sizeof(str) - 1];\
for(int i = 0; i < sizeof(str) - 1; i++) {\
if(i == 0) name[i] = (struct pixel_char){ { r, g, b, a }, { r_b, g_b, b_b, a_b }, str[i], {x, y}, flags, size };\
else name[i] = (struct pixel_char){ { r, g, b, a }, { r_b, g_b, b_b, a_b }, str[i], {name[i-1].position[0] + (size * ((game->resource_state.pixelfont_atlas[PIXELFONT_DEFAULT]->char_font_entries[name[i-1].value].width + 3) / 2 )), y}, flags, size  };\
}\

	string_to_pixel_char(chars, pixel_str, 3, 100 + 3 * game->resource_state.image_atlas[IMAGE_DIRT].width / 2 - 40, 100 + 3 * game->resource_state.image_atlas[IMAGE_DIRT].height / 2 - 10, PIXEL_CHAR_UNDERLINE_MASK | PIXEL_CHAR_SHADOW_MASK, 255, 255, 255, 255, 127, 0, 255, 255)

	chars[1].masks |= 2;

	renderer_backend_set_pixel_chars(game, chars, sizeof(pixel_str) - 1);

	struct renderer_rectangle rectangles[8];

	rectangles[0].image_index = IMAGE_MENU_BACKGROUND;
	rectangles[0].sampler_index = SAMPLER_SMOOTH;

	rectangles[0].u[0] = 0;
	rectangles[0].u[1] = 0;
	rectangles[0].u[2] = (float)game->application_state.window_extent.width / (float)(4 * game->resource_state.image_atlas[IMAGE_MENU_BACKGROUND].width);
	rectangles[0].u[3] = (float)game->application_state.window_extent.width / (float)(4 * game->resource_state.image_atlas[IMAGE_MENU_BACKGROUND].width);

	rectangles[0].v[0] = 0;
	rectangles[0].v[1] = (float)game->application_state.window_extent.height / (float)(4 * game->resource_state.image_atlas[IMAGE_MENU_BACKGROUND].height);;
	rectangles[0].v[2] = (float)game->application_state.window_extent.height / (float)(4 * game->resource_state.image_atlas[IMAGE_MENU_BACKGROUND].height);;
	rectangles[0].v[3] = 0;

	rectangles[0].x[0] = 0;
	rectangles[0].x[1] = 0;
	rectangles[0].x[2] = game->application_state.window_extent.width;
	rectangles[0].x[3] = game->application_state.window_extent.width;

	rectangles[0].y[0] = 0;
	rectangles[0].y[1] = game->application_state.window_extent.height;
	rectangles[0].y[2] = game->application_state.window_extent.height;
	rectangles[0].y[3] = 0;


	renderer_backend_set_rectangles(game, rectangles, 1);

	renderer_backend_render(game);

	return 0;
}

uint32_t renderer_update_resources(struct game_client* game) {

	renderer_backend_unload_resources(game);

	renderer_load_samplers(game);

	renderer_backend_load_resources(game);
	
	return 0;
}

uint32_t renderer_use_gpu(struct game_client* game, uint32_t gpu_index) {

	renderer_backend_use_gpu(game, gpu_index);

	return 0;
}

uint32_t renderer_load_samplers(struct game_client* game) {

	for (uint32_t i = 0; i < SAMPLERS_COUNT; i++) {

#define _check_samplers_token_string(token_name, parameter_name, options, options_tokens, default_option_index) {\
uint8_t buffer[64];\
enum key_value_return_type return_type = key_value_get_string(&game->resource_state.key_value_map_atlas[sampler_configurations_key_value_maps[i]], token_name, options_tokens[default_option_index], buffer, 64);\
\
if (return_type == KEY_VALUE_INFO_ADDED_PAIR) {\
	printf("[GAME RENDERER] Couldn't find token '%s' in sampler-keyvalue-file linked to token '%s'\n", token_name, resources_key_value_map_tokens[sampler_configurations_key_value_maps[i]]);\
	game->renderer_state.sampler_configurations[i].parameter_name = options[default_option_index];\
}\
else if (return_type == KEY_VALUE_INFO_CHANGED_TYPE) {\
	printf("[GAME RENDERER] Token '%s' wasn't of type STRING in sampler-keyvalue-file linked to token '%s'\n", token_name, resources_key_value_map_tokens[sampler_configurations_key_value_maps[i]]);\
	game->renderer_state.sampler_configurations[i].parameter_name = options[default_option_index];\
}\
else if (return_type == KEY_VALUE_ERROR_BUFFER_TOO_SMALL) {\
	printf("[GAME RENDERER] Token '%s' didn't match one of the allowed values { ", token_name); \
	for(uint32_t _wfw = 0; _wfw < sizeof(options); _wfw++) printf("%s ", options_tokens[_wfw]);\
	printf("} in sampler-keyvalue-file linked to token '%s'\n",  resources_key_value_map_tokens[sampler_configurations_key_value_maps[i]]);\
	game->renderer_state.sampler_configurations[i].parameter_name = options[default_option_index];\
}\
else {\
	uint32_t found = 0;\
	for(uint32_t _wfw = 0; _wfw < sizeof(options); _wfw++) {\
	if(strcmp(buffer, options_tokens[_wfw]) == 0) {\
	game->renderer_state.sampler_configurations[i].parameter_name = options[_wfw];\
	found = 1;\
	break;\
	}\
	}\
	if(found == 0) {\
	printf("[GAME RENDERER] Token '%s' didn't match one of the allowed values { ", token_name); \
	for(uint32_t _wfw = 0; _wfw < sizeof(options); _wfw++) printf("%s ", options_tokens[_wfw]);\
	printf("} in sampler-keyvalue-file linked to token '%s'\n",  resources_key_value_map_tokens[sampler_configurations_key_value_maps[i]]);\
	game->renderer_state.sampler_configurations[i].parameter_name = options[default_option_index];\
	}\
}\
}

#define _check_samplers_token_float(token_name, parameter_name, default_value) {\
float value;\
enum key_value_return_type return_type = key_value_get_float(&game->resource_state.key_value_map_atlas[sampler_configurations_key_value_maps[i]], token_name, default_value, &value);\
\
if (return_type == KEY_VALUE_INFO_ADDED_PAIR) {\
	printf("[GAME RENDERER] Couldn't find token '%s' in sampler-keyvalue-file linked to token '%s'\n", token_name, resources_key_value_map_tokens[sampler_configurations_key_value_maps[i]]);\
	game->renderer_state.sampler_configurations[i].parameter_name = default_value;\
}\
else if (return_type == KEY_VALUE_INFO_CHANGED_TYPE) {\
	printf("[GAME RENDERER] Token '%s' wasn't of type FLOAT in sampler-keyvalue-file linked to token '%s'\n", token_name, resources_key_value_map_tokens[sampler_configurations_key_value_maps[i]]);\
	game->renderer_state.sampler_configurations[i].parameter_name = default_value;\
}\
else {\
	game->renderer_state.sampler_configurations[i].parameter_name = value;\
}\
}

		uint8_t min_filter_options[] = { SAMPLING_LINEAR, SAMPLING_NEAREST };
		uint8_t* min_filter_options_tokens[] = { "LINEAR", "NEAREST" };
		_check_samplers_token_string("minFilter", min_filter, min_filter_options, min_filter_options_tokens, 0)

			uint8_t mag_filter_options[] = { SAMPLING_LINEAR, SAMPLING_NEAREST };
		uint8_t* mag_filter_options_tokens[] = { "LINEAR", "NEAREST" };
		_check_samplers_token_string("magFilter", mag_filter, mag_filter_options, mag_filter_options_tokens, 1)

			uint8_t  mip_map_mode_options[] = { SAMPLING_LINEAR, SAMPLING_NEAREST };
		uint8_t* mip_map_mode_options_tokens[] = { "LINEAR", "NEAREST" };
		_check_samplers_token_string("mipmapMode", mipmap_mode, mip_map_mode_options, mip_map_mode_options_tokens, 1)

			uint8_t  address_mode_u_options[] = { SAMPLING_REPEAT, SAMPLING_CLAMP_TO_EDGE };
		uint8_t* address_mode_u_options_tokens[] = { "REPEAT", "CLAMP_TO_EDGE" };
		_check_samplers_token_string("addressModeU", address_mode_u, address_mode_u_options, address_mode_u_options_tokens, 0)

			uint8_t  address_mode_v_options[] = { SAMPLING_REPEAT, SAMPLING_CLAMP_TO_EDGE };
		uint8_t* address_mode_v_options_tokens[] = { "REPEAT", "CLAMP_TO_EDGE" };
		_check_samplers_token_string("addressModeV", address_mode_v, address_mode_v_options, address_mode_v_options_tokens, 0)

			_check_samplers_token_float("mipLodBias", mip_lod_bias, 0.f)

			uint8_t  anisotropy_enable_options[] = { SAMPLING_DISABLE, SAMPLING_ENABLE };
		uint8_t* anisotropy_enable_options_tokens[] = { "DISABLE", "ENABLE" };
		_check_samplers_token_string("anisotropyEnable", anisotropy_enable, anisotropy_enable_options, anisotropy_enable_options_tokens, 0)

			_check_samplers_token_float("maxAnisotropy", max_anisotropy, 0.f)

			uint8_t  compare_enable_options[] = { SAMPLING_DISABLE, SAMPLING_ENABLE };
		uint8_t* compare_enable_options_tokens[] = { "DISABLE", "ENABLE" };
		_check_samplers_token_string("compareEnable", compare_enable, compare_enable_options, compare_enable_options_tokens, 0)

			uint8_t  compare_op_options[] = {
				SAMPLING_COMPARE_NEVER,
				SAMPLING_COMPARE_ALWAYS,
				SAMPLING_COMPARE_LESS,
				SAMPLING_COMPARE_EQUAL,
				SAMPLING_COMPARE_GREATER,
				SAMPLING_COMPARE_LESS_EQUAL,
				SAMPLING_COMPARE_GREATER_EQUAL,
				SAMPLING_COMPARE_NOT_EQUAL
		};
		uint8_t* compare_op_options_tokens[] = {
			"NEVER",
			"ALWAYS",
			"LESS",
			"EQUAL",
			"GREATER",
			"LESS_EQUAL",
			"GREATER_EQUAL",
			"NOT_EQUAL"
		};
		_check_samplers_token_string("compareOp", compare_op, compare_op_options, compare_op_options_tokens, 0)

			_check_samplers_token_float("minLod", min_lod, 0.f)

			_check_samplers_token_float("maxLod", max_lod, 16.f)


	}

}