#include "../renderer_backend.h"

#include "client/game_client.h"

#include "client/game_client.h"
#include "general/rendering/rendering_window.h"

#include "renderer_device.c"
#include "renderer_instance.c"
#include "vulkan_resources.c"
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


	pixel_char_renderer_new(&game->renderer_state.backend.pcr, game->renderer_state.backend.device, game->renderer_state.backend.gpu, game->renderer_state.backend.window_render_pass);

	return 0;
}


uint32_t renderer_backend_destroy(struct game_client* game) {

	pixel_char_renderer_destroy(&game->renderer_state.backend.pcr);

	if (game->renderer_state.backend.device != 0) renderer_backend_device_destroy(game);

	renderer_backend_instance_destroy(game);

	return 0;
}

uint32_t renderer_backend_use_gpu(struct game_client* game, uint32_t gpu_index) {
	if (gpu_index >= game->application_state.machine_info.gpu_count) return 1;
	if (game->application_state.machine_info.gpus[gpu_index].usable == 0) return 2;

	renderer_backend_unload_resources(game);

	pixel_char_renderer_destroy(&game->renderer_state.backend.pcr);

	renderer_backend_device_destroy(game);
	renderer_backend_device_create(game, gpu_index);

	pixel_char_renderer_new(&game->renderer_state.backend.pcr, game->renderer_state.backend.device, game->renderer_state.backend.gpu, game->renderer_state.backend.window_render_pass);

	renderer_backend_load_resources(game);

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