#include "graphics.h"

#include "client/game_client.h"

uint32_t graphics_device_create(struct game_client* game, uint32_t gpu_index) {

	game->graphics_state.queue_index = game->graphics_state.gpu_queue_indices[gpu_index];
	game->graphics_state.gpu = game->graphics_state.gpus[gpu_index];
	game->graphics_state.surface_format = game->graphics_state.gpu_surface_formats[gpu_index];

	float queue_priority = 1.f;

	VkDeviceQueueCreateInfo queue_info = { 0 };
	queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info.queueFamilyIndex = game->graphics_state.queue_index;
	queue_info.queueCount = 1;
	queue_info.pQueuePriorities = &queue_priority;

	char* device_extentions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkDeviceCreateInfo device_info = { 0 };
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.pQueueCreateInfos = &queue_info;
	device_info.queueCreateInfoCount = 1;
	device_info.ppEnabledExtensionNames = device_extentions;
	device_info.enabledExtensionCount = 1;

	VKCall(vkCreateDevice(game->graphics_state.gpu, &device_info, 0, &game->graphics_state.device));

	rendering_window_create_rendering_pass(game->graphics_state.device, game->graphics_state.surface_format, &game->graphics_state.window_render_pass);

	rendering_window_swapchain_create(
		game->application_state.window, 
		game->graphics_state.gpu, 
		game->graphics_state.device, 
		game->graphics_state.surface_format, 
		game->graphics_state.window_render_pass
	);

	VKCall(command_pool_create(game->graphics_state.device, game->graphics_state.queue_index, &game->graphics_state.command_pool));
	VKCall(command_buffer_allocate(game->graphics_state.device, game->graphics_state.command_pool, &game->graphics_state.cmd));

	VKCall(semaphore_create(game->graphics_state.device, &game->graphics_state.aquire_semaphore));
	VKCall(semaphore_create(game->graphics_state.device, &game->graphics_state.submit_semaphore));
	VKCall(fence_create(game->graphics_state.device, &game->graphics_state.img_available_fence));

	VKCall(rendering_memory_manager_new(
		game->graphics_state.device, 
		game->graphics_state.gpu, 
		game->graphics_state.queue, 
		game->graphics_state.command_pool, 
		&game->graphics_state.rmm
	));

	return 0;
}

uint32_t graphics_device_destroy(struct game_client* game) {

	VKCall(rendering_memory_manager_destroy(&game->graphics_state.rmm));

	vkDestroySemaphore(game->graphics_state.device, game->graphics_state.submit_semaphore, 0);
	vkDestroySemaphore(game->graphics_state.device, game->graphics_state.aquire_semaphore, 0);
	vkDestroyFence(game->graphics_state.device, game->graphics_state.img_available_fence, 0);
	vkDestroyRenderPass(game->graphics_state.device, game->graphics_state.window_render_pass, 0);
	vkFreeCommandBuffers(game->graphics_state.device, game->graphics_state.command_pool, 1, &game->graphics_state.cmd);
	vkDestroyCommandPool(game->graphics_state.device, game->graphics_state.command_pool, 0);

	rendering_window_swapchain_destroy(game->application_state.window);

	vkDestroyDevice(game->graphics_state.device, 0);

	return 0;
}