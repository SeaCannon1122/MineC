#include "renderer.h"

#include "client/game_client.h"

#include <stdio.h>
#include <string.h>

#include <math.h>
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

	char* pixel_str[] = {
		"This is a very long text.",
		"Well, at least if you compare it to what else i have written to this screen so far.",
		"Objectively speaking, this text is actually quite small but still large enough for it to not fit in a singular line!",
		"Hello World! WHat are yall doing on this fine evening? I'm just coding :) :=> ^-^",
		"rectangles[0].u[2] = (float)game->application_state.window_extent.width / (float)(4 * game->resource_state.image_atlas[IMAGE_MENU_BACKGROUND].width);",
		"https://www.youtube.com/watch?v=tLhfuc_KCpM",
		"Amphipoea oculea, the ear moth, is a moth of the family Noctuidae. It was first described by",
		"Carl Linnaeus in 1761 and it is found in most of the Palearctic realm.[citation needed] It is",
		"The wingspan is 29-34 mm. Forewing pale or dark ferruginous brown; the veins brown; inner and outer lines",
		"double, brown, wide apart; the inner curved outwards between, and toothed inwards on, the veins; the outer with the inner arm thin, lunulate-dentate, ",
		"the outer thick, continuous and parallel; a thick dark median shade running between the ",
		"stigmata; submarginal line indistinct, waved, angled on vein 7, above which it is preceded by a ",
		"dark costal patch; orbicular stigma rounded, orange, with a brown ring; reniform white, with the",
		"veins across it brown and containing on the discocellular a brown-outlined lunule, of which the",
		"centre is yellowish; the colour with brown outline; hindwing fuscous grey, paler towards base; ",
	};

#define string_to_pixel_char(name, str, size, x, y, flags, r, g, b, a, r_b, g_b, b_b, a_b) struct pixel_char name[300];\
for(int i = 0; i < strlen(str); i++) {\
if(i == 0) name[i] = (struct pixel_char){ { r, g, b, a }, { r_b, g_b, b_b, a_b }, str[i], {x, y}, flags, size };\
else name[i] = (struct pixel_char){ { r, g, b, a }, { r_b, g_b, b_b, a_b }, str[i], {name[i-1].position[0] + (size * ((game->resource_state.pixelfont_atlas[PIXELFONT_DEFAULT]->char_font_entries[name[i-1].value].width + 3) / 2 )), y}, flags, size  };\
}\

	for (int j = 0; j < 15; j++) {

		string_to_pixel_char(chars, pixel_str[j], 4, 50, 50 + j * 50, PIXELFONT_DEFAULT | PIXEL_CHAR_SHADOW_MASK | (j % 2 == 1 ? PIXEL_CHAR_UNDERLINE_MASK : 0) | PIXEL_CHAR_BACKGROUND_MASK, 255, 255, 255, 255, 100, 100, 100, 180)

		renderer_backend_add_pixel_chars(game, chars, strlen(pixel_str[j]));

	}

	
	double time = get_time();

	struct renderer_rectangle rectangles[8];

	rectangles[0].image_index = 0x00ff00ff;//IMAGE_MENU_BACKGROUND;
	rectangles[0].sampler_index = -1;// SAMPLER_SMOOTH;



	float time_dx = -fmod(time / 3000.f, 1.f);
	float time_dy = -fmod(time / 3000.f, 1.f);

	rectangles[0].u[0] = time_dx;
	rectangles[0].u[1] = time_dx;
	rectangles[0].u[2] = (float)game->application_state.window_extent.width / (float)(4 * game->resource_state.image_atlas[IMAGE_MENU_BACKGROUND].width) + time_dx;
	rectangles[0].u[3] = (float)game->application_state.window_extent.width / (float)(4 * game->resource_state.image_atlas[IMAGE_MENU_BACKGROUND].width) + time_dx;

	rectangles[0].v[0] = time_dy;
	rectangles[0].v[1] = (float)game->application_state.window_extent.height / (float)(4 * game->resource_state.image_atlas[IMAGE_MENU_BACKGROUND].height) + time_dy;
	rectangles[0].v[2] = (float)game->application_state.window_extent.height / (float)(4 * game->resource_state.image_atlas[IMAGE_MENU_BACKGROUND].height) + time_dy;
	rectangles[0].v[3] = time_dy;

	rectangles[0].x[0] = 0;
	rectangles[0].x[1] = 0;
	rectangles[0].x[2] = game->application_state.window_extent.width;
	rectangles[0].x[3] = game->application_state.window_extent.width;

	rectangles[0].y[0] = 0;
	rectangles[0].y[1] = game->application_state.window_extent.height;
	rectangles[0].y[2] = game->application_state.window_extent.height;
	rectangles[0].y[3] = 0;


	renderer_backend_add_rectangles(game, rectangles, 1);

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