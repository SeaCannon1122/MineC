#include "../renderer_backend.h"

uint32_t renderer_backend_device_create(struct game_client* game, uint32_t gpu_index) {

	game->renderer_state.backend.queue_index = game->renderer_state.backend.gpu_queue_indices[gpu_index];
	game->renderer_state.backend.gpu = game->renderer_state.backend.gpus[gpu_index];
	game->renderer_state.backend.surface_format = game->renderer_state.backend.gpu_surface_formats[gpu_index];

	float queue_priority = 1.f;

	VkDeviceQueueCreateInfo queue_info = { 0 };
	queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info.queueFamilyIndex = game->renderer_state.backend.queue_index;
	queue_info.queueCount = 1;
	queue_info.pQueuePriorities = &queue_priority;

	char* device_extentions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
		VK_KHR_MAINTENANCE3_EXTENSION_NAME,
	};

	VkPhysicalDeviceVulkan12Features features12 = { 0 };
	features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	features12.runtimeDescriptorArray = VK_TRUE;
	features12.descriptorIndexing = VK_TRUE;

	VkDeviceCreateInfo device_info = { 0 };
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.pQueueCreateInfos = &queue_info;
	device_info.queueCreateInfoCount = 1;
	device_info.ppEnabledExtensionNames = device_extentions;
	device_info.enabledExtensionCount = 3;
	device_info.pNext = &features12;

	VKCall(vkCreateDevice(game->renderer_state.backend.gpu, &device_info, 0, &game->renderer_state.backend.device));
	vkGetDeviceQueue(game->renderer_state.backend.device, game->renderer_state.backend.queue_index, 0, &game->renderer_state.backend.queue);

	rendering_window_create_rendering_pass(game->renderer_state.backend.device, game->renderer_state.backend.surface_format, &game->renderer_state.backend.window_render_pass);

	rendering_window_swapchain_create(
		game->application_state.window,
		game->renderer_state.backend.gpu,
		game->renderer_state.backend.device,
		game->renderer_state.backend.surface_format,
		VK_PRESENT_MODE_FIFO_KHR,
		game->renderer_state.backend.window_render_pass
	);

	VKCall(command_pool_create(game->renderer_state.backend.device, game->renderer_state.backend.queue_index, &game->renderer_state.backend.command_pool));
	VKCall(command_buffer_allocate(game->renderer_state.backend.device, game->renderer_state.backend.command_pool, &game->renderer_state.backend.cmd));

	VKCall(semaphore_create(game->renderer_state.backend.device, &game->renderer_state.backend.aquire_semaphore));
	VKCall(semaphore_create(game->renderer_state.backend.device, &game->renderer_state.backend.submit_semaphore));
	VKCall(fence_create(game->renderer_state.backend.device, &game->renderer_state.backend.queue_fence));

	VKCall(rendering_memory_manager_new(
		game->renderer_state.backend.device,
		game->renderer_state.backend.gpu,
		game->renderer_state.backend.queue,
		game->renderer_state.backend.queue_index,
		10000000,
		&game->renderer_state.backend.rmm
	));

	return 0;
}

uint32_t renderer_backend_device_destroy(struct game_client* game) {

	VKCall(rendering_memory_manager_destroy(&game->renderer_state.backend.rmm));

	vkDestroySemaphore(game->renderer_state.backend.device, game->renderer_state.backend.submit_semaphore, 0);
	vkDestroySemaphore(game->renderer_state.backend.device, game->renderer_state.backend.aquire_semaphore, 0);
	vkDestroyFence(game->renderer_state.backend.device, game->renderer_state.backend.queue_fence, 0);
	vkDestroyRenderPass(game->renderer_state.backend.device, game->renderer_state.backend.window_render_pass, 0);
	vkFreeCommandBuffers(game->renderer_state.backend.device, game->renderer_state.backend.command_pool, 1, &game->renderer_state.backend.cmd);
	vkDestroyCommandPool(game->renderer_state.backend.device, game->renderer_state.backend.command_pool, 0);

	rendering_window_swapchain_destroy(game->application_state.window);

	vkDestroyDevice(game->renderer_state.backend.device, 0);

	return 0;
}