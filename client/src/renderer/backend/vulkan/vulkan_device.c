#include "vulkan_backend.h"

uint32_t vulkan_device_create(struct minec_client* game, uint32_t gpu_index) {

	game->application_state.machine_info.gpu_index = gpu_index;

	game->renderer_state.backend.queue_index = game->renderer_state.backend.gpu_queue_indices[gpu_index];
	game->renderer_state.backend.gpu = game->renderer_state.backend.gpus[gpu_index];
	game->renderer_state.backend.surface_format = game->renderer_state.backend.gpu_surface_formats[gpu_index];

	VKCall(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(game->renderer_state.backend.gpu, game->renderer_state.backend.surface, &game->renderer_state.backend.surface_capabilities));

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

	VkPhysicalDeviceFeatures enabled_features = { 0 };
	enabled_features.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo device_info = { 0 };
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.pQueueCreateInfos = &queue_info;
	device_info.queueCreateInfoCount = 1;
	device_info.ppEnabledExtensionNames = (const char* const*) device_extentions;
	device_info.enabledExtensionCount = 3;
	device_info.pEnabledFeatures = &enabled_features;
	device_info.pNext = &features12;

	VKCall(vkCreateDevice(game->renderer_state.backend.gpu, &device_info, 0, &game->renderer_state.backend.device));
	vkGetDeviceQueue(game->renderer_state.backend.device, game->renderer_state.backend.queue_index, 0, &game->renderer_state.backend.queue);

	VkCommandPoolCreateInfo pool_info = { 0 };
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = game->renderer_state.backend.queue_index;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VKCall(vkCreateCommandPool(game->renderer_state.backend.device, &pool_info, 0, &game->renderer_state.backend.command_pool));

	VkCommandBufferAllocateInfo alloc_info = { 0 };
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandBufferCount = 1;
	alloc_info.commandPool = game->renderer_state.backend.command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VKCall(vkAllocateCommandBuffers(game->renderer_state.backend.device, &alloc_info, &game->renderer_state.backend.cmd));

	VkSemaphoreCreateInfo sema_info = { 0 };
	sema_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VKCall(vkCreateSemaphore(game->renderer_state.backend.device, &sema_info, 0, &game->renderer_state.backend.aquire_semaphore));
	VKCall(vkCreateSemaphore(game->renderer_state.backend.device, &sema_info, 0, &game->renderer_state.backend.submit_semaphore));

	VkFenceCreateInfo fence_info = { 0 };
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VKCall(vkCreateFence(game->renderer_state.backend.device, &fence_info, 0, &game->renderer_state.backend.queue_fence));

	game->renderer_state.backend.queue_used = 0;

	return 0;
}

uint32_t vulkan_device_destroy(struct minec_client* game) {

	vkDeviceWaitIdle(game->renderer_state.backend.device);

	vkDestroySemaphore(game->renderer_state.backend.device, game->renderer_state.backend.submit_semaphore, 0);
	vkDestroySemaphore(game->renderer_state.backend.device, game->renderer_state.backend.aquire_semaphore, 0);
	vkDestroyFence(game->renderer_state.backend.device, game->renderer_state.backend.queue_fence, 0);
	vkFreeCommandBuffers(game->renderer_state.backend.device, game->renderer_state.backend.command_pool, 1, &game->renderer_state.backend.cmd);
	vkDestroyCommandPool(game->renderer_state.backend.device, game->renderer_state.backend.command_pool, 0);

	vkDestroyDevice(game->renderer_state.backend.device, 0);

	return 0;
}