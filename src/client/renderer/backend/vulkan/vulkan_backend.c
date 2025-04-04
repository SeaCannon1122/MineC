#include "vulkan_backend.h"

#include "client/game_client.h"

#include "vulkan_device.c"
#include "vulkan_instance.c"
#include "vulkan_device_swapchain_and_framebuffers.c"
#include "vulkan_device_renderpasses.c"
#include "vulkan_device_resources.c"
#include "vulkan_device_resources_rectangles.c"

uint32_t vulkan_backend_create(struct game_client* game) {

	vulkan_instance_create(game);

	game->renderer_state.backend.device = 0;
	for (int i = 0; i < game->application_state.machine_info.gpu_count; i++) if (game->application_state.machine_info.gpus[i].usable) {

		vulkan_device_create(game, i);
		break;
	}

	if (game->renderer_state.backend.device == 0) {
		vulkan_instance_destroy(game);
		return 1;
	}

	vulkan_device_renderpasses_create(game);

	vulkan_device_swapchain_and_framebuffers_create(game);
	
	game->renderer_state.backend.resources_created = 0;

	return 0;
}


uint32_t vulkan_backend_destroy(struct game_client* game) {

	vulkan_device_swapchain_and_framebuffers_destroy(game);

	vulkan_device_renderpasses_destroy(game);

	vulkan_device_destroy(game);

	vulkan_instance_destroy(game);

	return 0;
}

uint32_t vulkan_backend_load_resources(struct game_client* game) {

	game->renderer_state.backend.resources_created = 1;

	vulkan_device_resources_create(game);

	return 0;
}

uint32_t vulkan_backend_unload_resources(struct game_client* game) {

	game->renderer_state.backend.resources_created = 0;

	vulkan_device_resources_destroy(game);

	return 0;
}

uint32_t vulkan_backend_use_gpu(struct game_client* game, uint32_t gpu_index) {
	if (gpu_index >= game->application_state.machine_info.gpu_count) return 1;
	if (game->application_state.machine_info.gpus[gpu_index].usable == 0) return 2;

	uint32_t resources = game->renderer_state.backend.resources_created;

	if (resources) vulkan_device_resources_destroy(game);

	vulkan_device_swapchain_and_framebuffers_destroy(game);

	vulkan_device_renderpasses_destroy(game);

	vulkan_device_destroy(game);
	vulkan_device_create(game, gpu_index);

	vulkan_device_renderpasses_create(game);

	vulkan_device_swapchain_and_framebuffers_create(game);

	if (resources) vulkan_device_resources_create(game);

	return 0;
}

uint32_t vulkan_backend_resize(struct game_client* game) {

	vulkan_device_swapchain_and_framebuffers_destroy(game);
	vulkan_device_swapchain_and_framebuffers_create(game);

	return 0;
}

uint32_t vulkan_backend_render(struct game_client* game) {

	if (game->renderer_state.backend.queue_used) VKCall(vkWaitForFences(game->renderer_state.backend.device, 1, &game->renderer_state.backend.queue_fence, VK_TRUE, UINT64_MAX));

	VKCall(vkResetFences(game->renderer_state.backend.device, 1, &game->renderer_state.backend.queue_fence));

	game->renderer_state.backend.queue_used = 0;

	uint32_t swapchain_image_index;
	if (vkAcquireNextImageKHR(game->renderer_state.backend.device, game->renderer_state.backend.swapchain, 0, game->renderer_state.backend.aquire_semaphore, 0, &swapchain_image_index) != VK_SUCCESS) {
		printf("[RENDERER BACKEND] Error aquireing next swapchain image\n");
		
		return 1;
	}

	VKCall(vkResetCommandBuffer(game->renderer_state.backend.cmd, 0));

	VkCommandBufferBeginInfo begin_info = { 0 };
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VKCall(vkBeginCommandBuffer(game->renderer_state.backend.cmd, &begin_info));

	VkExtent2D screen_size;
	screen_size.width = game->application_state.main_window.width;
	screen_size.height = game->application_state.main_window.height;

	VkRenderPassBeginInfo renderpass_begin_info = { 0 };
	renderpass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpass_begin_info.renderPass = game->renderer_state.backend.window_render_pass;
	renderpass_begin_info.renderArea.extent = screen_size;
	renderpass_begin_info.framebuffer = game->renderer_state.backend.framebuffers[swapchain_image_index];

	vkCmdBeginRenderPass(game->renderer_state.backend.cmd, &renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);


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

	game->renderer_state.backend.rectangles_count = 0;

	//pixel_chars
	pixelchar_renderer_vk_cmd_render(&game->renderer_state.backend.pcr, game->renderer_state.backend.cmd, screen_size);

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

	

	VkPresentInfoKHR present_info = { 0 };
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.pSwapchains = &game->renderer_state.backend.swapchain;
	present_info.swapchainCount = 1;
	present_info.pImageIndices = &swapchain_image_index;
	present_info.pWaitSemaphores = &game->renderer_state.backend.submit_semaphore;
	present_info.waitSemaphoreCount = 1;

	VKCall(vkQueuePresentKHR(game->renderer_state.backend.queue, &present_info));

	game->renderer_state.backend.queue_used = 1;

	return 0;
}

uint32_t vulkan_backend_add_pixel_chars(struct game_client* game, struct pixel_char* chars, uint32_t chars_count) {

	pixelchar_renderer_vk_add_chars(&game->renderer_state.backend.pcr, chars, chars_count);

	return 0;
};