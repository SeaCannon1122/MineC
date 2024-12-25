#include "resources.h"

#include <stdio.h>

#include "client/game_client.h"


uint32_t resources_create(struct game_client* game, uint8_t* resource_path) {
	resource_manager_new(&game->resource_state.resource_manager, resource_path);

	sprintf(game->resource_state.resource_manager_root_path, resource_path);

	for (uint32_t i = 0; i < RESOURCES_IMAGES_COUNT; i++) {
		if (resource_manager_get_image(&game->resource_state.resource_manager, resources_image_tokens[i], &game->resource_state.image_atlas[i])) {

			printf("[GAME RESOURCES] Couldn't find image matching token '%s'\n", resources_image_tokens[i]);
			game->resource_state.image_atlas[i].data = default_image;
			game->resource_state.image_atlas[i].width = default_image_width;
			game->resource_state.image_atlas[i].height = default_image_height;

		}
	}

	game->resource_state.fallback_pixelfont = NULL;

	for (uint32_t i = 0; i < RESOURCES_PIXELFONTS_COUNT; i++) {

		struct resource_manager_binary binary;

		if (resource_manager_get_binary(&game->resource_state.resource_manager, resources_pixelfont_tokens[i], &binary)) {

			printf("[GAME RESOURCES] Couldn't find pixelfont matching token '%s'\n", resources_pixelfont_tokens[i]);

			if (game->resource_state.fallback_pixelfont == NULL) game->resource_state.fallback_pixelfont = calloc(1, sizeof(struct pixel_font));

			game->resource_state.pixelfont_atlas[i] = game->resource_state.fallback_pixelfont;

		}

		else {
			game->resource_state.pixelfont_atlas[i] = calloc(1, sizeof(struct pixel_font));

			memcpy(game->resource_state.pixelfont_atlas[i], binary.data, binary.size);

		}
	}

	for (uint32_t i = 0; i < RESOURCES_SAMPLERS_COUNT; i++) {

		void* sampler_map;

		if (resource_manager_get_key_value_map(&game->resource_state.resource_manager, resources_sampler_tokens[i], &sampler_map)) {

			printf("[GAME RESOURCES] Couldn't find sampler-key_value_map matching token '%s'\n", resources_sampler_tokens[i]);

			game->resource_state.sampler_atlas[i] = (struct sampler_configuration){
				SAMPLING_LINEAR,
				SAMPLING_NEAREST, 
				SAMPLING_NEAREST, 
				SAMPLING_REPEAT, 
				SAMPLING_REPEAT, 
				0.f, 
				SAMPLING_DISABLE, 
				0.f, 
				SAMPLING_DISABLE, 
				SAMPLING_COMPARE_NEVER,
				0.f,
				16.f
			};

		}

		else {

#define _resource_check_samplers_token_string(token_name, parameter_name, options, options_tokens, default_option_index) {\
uint8_t buffer[64];\
enum key_value_return_type return_type = key_value_get_string(&sampler_map, token_name, options_tokens[default_option_index], buffer, 64);\
\
if (return_type == KEY_VALUE_INFO_ADDED_PAIR) {\
	printf("[GAME RESOURCES] Couldn't find token '%s' in sampler file linked to token '%s'\n", token_name, resources_sampler_tokens[i]);\
	game->resource_state.sampler_atlas[i].parameter_name = options[default_option_index];\
}\
else if (return_type == KEY_VALUE_INFO_CHANGED_TYPE) {\
	printf("[GAME RESOURCES] Token '%s' wasn't of type STRING in sampler file linked to token '%s'\n", token_name, resources_sampler_tokens[i]);\
	game->resource_state.sampler_atlas[i].parameter_name = options[default_option_index];\
}\
else if (return_type == KEY_VALUE_ERROR_BUFFER_TOO_SMALL) {\
	printf("[GAME RESOURCES] Token '%s' didn't match one of the allowed values { ", token_name); \
	for(uint32_t _wfw = 0; _wfw < sizeof(options); _wfw++) printf("%s ", options_tokens[_wfw]);\
	printf("} in sampler file linked to token '%s'\n",  resources_sampler_tokens[i]);\
	game->resource_state.sampler_atlas[i].parameter_name = options[default_option_index];\
}\
else {\
	uint32_t found = 0;\
	for(uint32_t _wfw = 0; _wfw < sizeof(options); _wfw++) {\
	if(strcmp(buffer, options_tokens[_wfw]) == 0) {\
	game->resource_state.sampler_atlas[i].parameter_name = options[_wfw];\
	found = 1;\
	break;\
	}\
	}\
	if(found == 0) {\
	printf("[GAME RESOURCES] Token '%s' didn't match one of the allowed values { ", token_name); \
	for(uint32_t _wfw = 0; _wfw < sizeof(options); _wfw++) printf("%s ", options_tokens[_wfw]);\
	printf("} in sampler file linked to token '%s'\n",  resources_sampler_tokens[i]);\
	game->resource_state.sampler_atlas[i].parameter_name = options[default_option_index];\
	}\
}\
}

#define _resource_check_samplers_token_float(token_name, parameter_name, default_value) {\
float value;\
enum key_value_return_type return_type = key_value_get_float(&sampler_map, token_name, default_value, &value);\
\
if (return_type == KEY_VALUE_INFO_ADDED_PAIR) {\
	printf("[GAME RESOURCES] Couldn't find token '%s' in sampler file linked to token '%s'\n", token_name, resources_sampler_tokens[i]);\
	game->resource_state.sampler_atlas[i].parameter_name = default_value;\
}\
else if (return_type == KEY_VALUE_INFO_CHANGED_TYPE) {\
	printf("[GAME RESOURCES] Token '%s' wasn't of type FLOAT in sampler file linked to token '%s'\n", token_name, resources_sampler_tokens[i]);\
	game->resource_state.sampler_atlas[i].parameter_name = default_value;\
}\
else {\
	game->resource_state.sampler_atlas[i].parameter_name = value;\
}\
}

			uint8_t min_filter_options[] = { SAMPLING_LINEAR, SAMPLING_NEAREST };
			uint8_t* min_filter_options_tokens[] = { "LINEAR", "NEAREST" };
			_resource_check_samplers_token_string("minFilter", min_filter, min_filter_options, min_filter_options_tokens, 0)
		
			uint8_t mag_filter_options[] = { SAMPLING_LINEAR, SAMPLING_NEAREST };
			uint8_t* mag_filter_options_tokens[] = { "LINEAR", "NEAREST" };
			_resource_check_samplers_token_string("magFilter", mag_filter, mag_filter_options, mag_filter_options_tokens, 1)

			uint8_t  mip_map_mode_options[] = { SAMPLING_LINEAR, SAMPLING_NEAREST };
			uint8_t* mip_map_mode_options_tokens[] = { "LINEAR", "NEAREST" };
			_resource_check_samplers_token_string("mipmapMode", mipmap_mode, mip_map_mode_options, mip_map_mode_options_tokens, 1)

			uint8_t  address_mode_u_options[] = { SAMPLING_REPEAT, SAMPLING_CLAMP_TO_EDGE };
			uint8_t* address_mode_u_options_tokens[] = { "REPEAT", "CLAMP_TO_EDGE" };
			_resource_check_samplers_token_string("addressModeU", address_mode_u, address_mode_u_options, address_mode_u_options_tokens, 0)

			uint8_t  address_mode_v_options[] = { SAMPLING_REPEAT, SAMPLING_CLAMP_TO_EDGE };
			uint8_t* address_mode_v_options_tokens[] = { "REPEAT", "CLAMP_TO_EDGE" };
			_resource_check_samplers_token_string("addressModeV", address_mode_v, address_mode_v_options, address_mode_v_options_tokens, 0)

			_resource_check_samplers_token_float("mipLodBias", mip_lod_bias, 0.f)

			uint8_t  anisotropy_enable_options[] = { SAMPLING_DISABLE, SAMPLING_ENABLE };
			uint8_t* anisotropy_enable_options_tokens[] = { "DISABLE", "ENABLE" };
			_resource_check_samplers_token_string("anisotropyEnable", anisotropy_enable, anisotropy_enable_options, anisotropy_enable_options_tokens, 0)

			_resource_check_samplers_token_float("maxAnisotropy", max_anisotropy, 0.f)

			uint8_t  compare_enable_options[] = { SAMPLING_DISABLE, SAMPLING_ENABLE };
			uint8_t* compare_enable_options_tokens[] = { "DISABLE", "ENABLE" };
			_resource_check_samplers_token_string("compareEnable", compare_enable, compare_enable_options, compare_enable_options_tokens, 0)

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
			_resource_check_samplers_token_string("compareOp", compare_op, compare_op_options, compare_op_options_tokens, 0)

			_resource_check_samplers_token_float("minLod", min_lod, 0.f)

			_resource_check_samplers_token_float("maxLod", max_lod, 16.f)

			free(sampler_map);
		}

	}

	return 0;
}


uint32_t resources_destroy(struct game_client* game) {

	if (game->resource_state.fallback_pixelfont != NULL) free(game->resource_state.fallback_pixelfont);
	
	resource_manager_destroy(&game->resource_state.resource_manager);

	return 0;
}

uint32_t resources_reload(struct game_client* game) {

	resources_destroy(game);
	resources_create(game, game->resource_state.resource_manager_root_path);

	return 0;
}