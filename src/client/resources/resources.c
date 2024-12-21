#include "resources.h"

#include <stdio.h>

#include "client/game_client.h"

uint32_t resources_create(struct game_client* game, uint32_t* resource_path) {
	if (resource_manager_new(&game->resource_state.resource_manager, resource_path) != 0) return 1;

	sprintf(game->resource_state.resource_manager_root_path, resource_path);

	game->resource_state.device_used = 0;

	for (uint32_t i = 0; i < RESOURCES_IMAGES_COUNT; i++) {
		game->resource_state.image_atlas[i] = resource_manager_get_image_index(&game->resource_state.resource_manager, resources_image_configurations[i].resource_token);

		if (game->resource_state.image_atlas[i] == 0 && strcmp(resources_image_configurations[i].resource_token, "default") != 0) {
			printf("[GAME RESOURCES] Couldn't find image matching token '%s'\n", resources_image_configurations[i].resource_token);
		}
	}
		
	for (uint32_t i = 0; i < RESOURCES_SHADERS_COUNT; i++) {
		int32_t shader_index = resource_manager_get_shader_index(&game->resource_state.resource_manager, resources_shader_tokens[i]);
		if (shader_index == -1) {
			printf("[GAME RESOURCES] [CRITICAL ERROR] Couldn't find shader matching token '%s'\n", resources_shader_tokens[i]);
			resource_manager_destroy(&game->resource_state.resource_manager);
			return 1;
		}
		game->resource_state.shader_atlas[i] = shader_index;
	}

	for (uint32_t i = 0; i < RESOURCES_PIXELFONTS_COUNT; i++) {
		int32_t pixelfont_index = resource_manager_get_shader_index(&game->resource_state.resource_manager, resources_pixelfont_tokens[i]);
		if (pixelfont_index == -1) {
			printf("[GAME RESOURCES] [CRITICAL ERROR] Couldn't find pixelfont matching token '%s'\n", resources_pixelfont_tokens[i]);
			resource_manager_destroy(&game->resource_state.resource_manager);
			return 1;
		}
		game->resource_state.pixelfont_atlas[i] = pixelfont_index;
	}

	resources_switch_use_vulkan_device(game);

	return 0;
}

uint32_t resources_switch_use_vulkan_device(struct game_client* game) {

	game->resource_state.device_used = 1;

	resource_manager_use_vulkan_device(&game->resource_state.resource_manager, game->graphics_state.device, game->graphics_state.gpu, game->graphics_state.queue, game->graphics_state.queue_index);
	
	for (uint32_t i = 0; i < RESOURCES_SAMPLER_COUNT; i++) {

		VkSamplerCreateInfo sampler_create_info = { 0 };
		sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_create_info.magFilter = resources_sampler_configurations[i].filer_mag;
		sampler_create_info.minFilter = resources_sampler_configurations[i].filer_min;
		sampler_create_info.addressModeU = resources_sampler_configurations[i].address_mode_u;
		sampler_create_info.addressModeV = resources_sampler_configurations[i].address_mode_v;
		sampler_create_info.anisotropyEnable = resources_sampler_configurations[i].anisotropy_enable;
		sampler_create_info.maxAnisotropy = resources_sampler_configurations[i].max_anisotropy;
		sampler_create_info.compareEnable = resources_sampler_configurations[i].compare_enable;
		sampler_create_info.compareOp = resources_sampler_configurations[i].compare_op;

		VKCall(vkCreateSampler(game->graphics_state.device, &sampler_create_info, 0, &game->resource_state.samplers[i]));

	}

	for (uint32_t i = 0; i < RESOURCES_IMAGES_COUNT; i++) {
		game->resource_state.descriptor_image_infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		game->resource_state.descriptor_image_infos[i].imageView = game->resource_state.resource_manager.images[game->resource_state.image_atlas[i]].image_view;
		game->resource_state.descriptor_image_infos[i].sampler = game->resource_state.samplers[resources_image_configurations[i].sampler_id];
	}

	

	return 0;
}

uint32_t resources_update_descriptor_set(struct game_client* game, VkDescriptorSet dst_set, uint32_t dst_binding, uint32_t dst_array_element) {

	VkWriteDescriptorSet descriptor_set_update_write = { 0 };
	descriptor_set_update_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_set_update_write.dstSet = dst_set;
	descriptor_set_update_write.dstBinding = dst_binding;
	descriptor_set_update_write.dstArrayElement = dst_array_element;
	descriptor_set_update_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptor_set_update_write.descriptorCount = RESOURCES_IMAGES_COUNT;
	descriptor_set_update_write.pImageInfo = game->resource_state.descriptor_image_infos;

	vkUpdateDescriptorSets(game->graphics_state.device, 1, &descriptor_set_update_write, 0, NULL);

	return 0;
}

uint32_t resources_switch_drop_vulkan_device(struct game_client* game) {

	game->resource_state.device_used = 0;

	resource_manager_drop_vulkan_device(&game->resource_state.resource_manager);

	for (uint32_t i = 0; i < RESOURCES_SAMPLER_COUNT; i++) {
		vkDestroySampler(game->graphics_state.device, game->resource_state.samplers[i], 0);
	}

	return 0;
}

uint32_t resources_destroy(struct game_client* game) {

	if (game->resource_state.device_used) resources_switch_drop_vulkan_device(game);

	resource_manager_destroy(&game->resource_state.resource_manager);

	return 0;
}

uint32_t resources_reload(struct game_client* game) {

	resources_destroy(game);
	resources_create(game, game->resource_state.resource_manager_root_path);

	return 0;
}