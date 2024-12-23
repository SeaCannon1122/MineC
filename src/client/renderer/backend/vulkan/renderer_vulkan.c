#include "../renderer_backend.h"

#include "client/game_client.h"

#include "client/game_client.h"
#include "general/rendering/rendering_window.h"
#include <malloc.h>

#include "renderer_device.c"
#include "renderer_instance.c"
#include "renderer_resources.c"
#include "vulkan_image.c"


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

	for (uint32_t i = 0; i < RENDERER_SAMPLING_CONFIGURATIONS_COUNT; i++) {
	
		VkSamplerCreateInfo sampler_info = { 0 };
		sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			
		if (renderer_sampling_configurations[i].min_filter == SAMPLING_NEAREST) sampler_info.minFilter = VK_FILTER_NEAREST;
		if (renderer_sampling_configurations[i].min_filter == SAMPLING_LINEAR) sampler_info.minFilter = VK_FILTER_LINEAR;

		if (renderer_sampling_configurations[i].mag_filter == SAMPLING_NEAREST) sampler_info.magFilter = VK_FILTER_NEAREST;
		if (renderer_sampling_configurations[i].mag_filter == SAMPLING_LINEAR) sampler_info.magFilter = VK_FILTER_LINEAR;

		if (renderer_sampling_configurations[i].mipmap_mode == SAMPLING_NEAREST) sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		if (renderer_sampling_configurations[i].mipmap_mode == SAMPLING_LINEAR) sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		if (renderer_sampling_configurations[i].address_mode_u == SAMPLING_REPEAT) sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		if (renderer_sampling_configurations[i].address_mode_u == SAMPLING_CLAMP_TO_EDGE) sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

		if (renderer_sampling_configurations[i].address_mode_v == SAMPLING_REPEAT) sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		if (renderer_sampling_configurations[i].address_mode_v == SAMPLING_CLAMP_TO_EDGE) sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

		sampler_info.mipLodBias = renderer_sampling_configurations[i].mip_lod_bias;

		if (renderer_sampling_configurations[i].anisotropy_enable == SAMPLING_ENABLE) sampler_info.anisotropyEnable = VK_TRUE;
		if (renderer_sampling_configurations[i].anisotropy_enable == SAMPLING_DISABLE) sampler_info.anisotropyEnable = VK_FALSE;

		sampler_info.maxAnisotropy = renderer_sampling_configurations[i].max_anisotropy;

		if (renderer_sampling_configurations[i].compare_enable == SAMPLING_ENABLE) sampler_info.compareEnable = VK_TRUE;
		if (renderer_sampling_configurations[i].compare_enable == SAMPLING_DISABLE) sampler_info.compareEnable = VK_FALSE;

		if (renderer_sampling_configurations[i].compare_op == SAMPLING_COMPARE_NEVER) sampler_info.compareOp = VK_COMPARE_OP_NEVER;
		if (renderer_sampling_configurations[i].compare_op == SAMPLING_COMPARE_ALWAYS) sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
		if (renderer_sampling_configurations[i].compare_op == SAMPLING_COMPARE_EQUAL) sampler_info.compareOp = VK_COMPARE_OP_EQUAL;
		if (renderer_sampling_configurations[i].compare_op == SAMPLING_COMPARE_LESS_EQUAL) sampler_info.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		if (renderer_sampling_configurations[i].compare_op == SAMPLING_COMPARE_LESS) sampler_info.compareOp = VK_COMPARE_OP_LESS;
		if (renderer_sampling_configurations[i].compare_op == SAMPLING_COMPARE_GREATER_EQUAL) sampler_info.compareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
		if (renderer_sampling_configurations[i].compare_op == SAMPLING_COMPARE_GREATER) sampler_info.compareOp = VK_COMPARE_OP_GREATER;
		if (renderer_sampling_configurations[i].compare_op == SAMPLING_COMPARE_NOT_EQUAL) sampler_info.compareOp = VK_COMPARE_OP_NOT_EQUAL;

		sampler_info.minLod = renderer_sampling_configurations[i].min_lod;
		sampler_info.maxLod = renderer_sampling_configurations[i].max_lod;

		VKCall(vkCreateSampler(game->renderer_state.backend.device, &sampler_info, 0, &game->renderer_state.backend.samplers[i]));

	}

	return 0;
}


uint32_t renderer_backend_destroy(struct game_client* game) {

	for (uint32_t i = 0; i < RENDERER_SAMPLING_CONFIGURATIONS_COUNT; i++) {
		vkDestroySampler(game->renderer_state.backend.device, game->renderer_state.backend.samplers[i], 0);
	}

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