#include "../renderer_backend.h"

#include "client/game_client.h"

#include "client/game_client.h"
#include "general/rendering/rendering_window.h"
#include <malloc.h>

#include "renderer_device.c"
#include "renderer_instance.c"
#include "renderer_resources.c"
#include "vulkan_image.c"
#include "renderer_rectangles.c"

uint32_t renderer_backend_create(struct game_client* game) {

	renderer_backend_instance_create(game);

	renderer_backend_list_gpus(game);

	game->renderer_state.backend.device = 0;
	for (int i = 0; i < game->application_state.machine_info.gpu_count; i++) if (game->application_state.machine_info.gpus[i].usable) {

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

	pixel_char_renderer_new(&game->renderer_state.backend.pcr, game->renderer_state.backend.device, game->renderer_state.backend.gpu, game->renderer_state.backend.window_render_pass);

	return 0;
}


uint32_t renderer_backend_destroy(struct game_client* game) {

	pixel_char_renderer_destroy(&game->renderer_state.backend.pcr);

	for (uint32_t i = 0; i < RENDERER_SAMPLING_CONFIGURATIONS_COUNT; i++) {
		vkDestroySampler(game->renderer_state.backend.device, game->renderer_state.backend.samplers[i], 0);
	}

	if (game->renderer_state.backend.device != 0) renderer_backend_device_destroy(game);

	renderer_backend_instance_destroy(game);

	return 0;
}

uint32_t renderer_backend_use_gpu(struct game_client* game, uint32_t gpu_index) {
	if (gpu_index >= game->application_state.machine_info.gpu_count) return 1;
	if (game->application_state.machine_info.gpus[gpu_index].usable == 0) return 2;

	renderer_backend_device_destroy(game);
	renderer_backend_device_create(game, gpu_index);

	return 0;
}

uint32_t renderer_backend_resize(struct game_client* game) {

	rendering_window_resize(game->application_state.window);

	return 0;
}

uint32_t renderer_backend_render(struct game_client* game) {

	VKCall(vkResetFences(game->renderer_state.backend.device, 1, &game->renderer_state.backend.queue_fence));

	VkRenderPassBeginInfo renderpass_begin_info = { 0 };
	rendering_window_renderpass_begin_info(game->application_state.window, &renderpass_begin_info, game->renderer_state.backend.window_render_pass, game->renderer_state.backend.aquire_semaphore);

	VKCall(vkResetCommandBuffer(game->renderer_state.backend.cmd, 0));

	VkCommandBufferBeginInfo begin_info = { 0 };
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VKCall(vkBeginCommandBuffer(game->renderer_state.backend.cmd, &begin_info));

	vkCmdBeginRenderPass(game->renderer_state.backend.cmd, &renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

	VkExtent2D screen_size;
	screen_size.width = game->application_state.window_extent.width;
	screen_size.height = game->application_state.window_extent.height;

	//rectangles
	if (game->renderer_state.backend.rectangles_pipeline_usable_bool != 0 && game->renderer_state.backend.rectangles_count != 0) {

		VkRect2D scissor = { 0 };
		scissor.extent = screen_size;

		VkViewport viewport = { 0 };
		viewport.width = screen_size.width;
		viewport.height = screen_size.height;
		viewport.maxDepth = 1.0f;

		vkCmdSetViewport(game->renderer_state.backend.cmd, 0, 1, &viewport);
		vkCmdSetScissor(game->renderer_state.backend.cmd, 0, 1, &scissor);

		vkCmdBindDescriptorSets(
			game->renderer_state.backend.cmd,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			game->renderer_state.backend.rectangles_pipeline_layout,
			0,
			1,
			&game->renderer_state.backend.images_descriptor_set,
			0,
			0
		);

		vkCmdBindPipeline(game->renderer_state.backend.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, game->renderer_state.backend.rectangles_pipeline);

		VkDeviceSize device_size = 0;
		vkCmdBindVertexBuffers(game->renderer_state.backend.cmd, 0, 1, &game->renderer_state.backend.rectangles_buffer, &device_size);

		vkCmdPushConstants(
			game->renderer_state.backend.cmd,
			game->renderer_state.backend.rectangles_pipeline_layout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(uint32_t) * 2,
			&screen_size
		);

		vkCmdDraw(game->renderer_state.backend.cmd, 6 * game->renderer_state.backend.rectangles_count, 1, 0, 0);

	}


	//pixel_chars
	pixel_char_renderer_render(&game->renderer_state.backend.pcr, game->renderer_state.backend.cmd, screen_size);

	vkCmdEndRenderPass(game->renderer_state.backend.cmd);

	VKCall(vkEndCommandBuffer(game->renderer_state.backend.cmd));

	VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submit_info = { 0 };
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pWaitDstStageMask = &wait_stage;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &game->renderer_state.backend.cmd;
	submit_info.pSignalSemaphores = &game->renderer_state.backend.submit_semaphore;
	submit_info.pWaitSemaphores = &game->renderer_state.backend.aquire_semaphore;
	submit_info.signalSemaphoreCount = 1;
	submit_info.waitSemaphoreCount = 1;


	VKCall(vkQueueSubmit(game->renderer_state.backend.queue, 1, &submit_info, game->renderer_state.backend.queue_fence));

	rendering_window_present_image(game->application_state.window, game->renderer_state.backend.queue, &game->renderer_state.backend.submit_semaphore, 1);

	VKCall(vkWaitForFences(game->renderer_state.backend.device, 1, &game->renderer_state.backend.queue_fence, VK_TRUE, UINT64_MAX));

	return 0;
}

uint32_t renderer_backend_set_pixel_chars(struct game_client* game, struct pixel_render_char* chars, uint32_t chars_count) {

	pixel_char_renderer_fill_chars(&game->renderer_state.backend.pcr, chars, chars_count);

	return 0;
}