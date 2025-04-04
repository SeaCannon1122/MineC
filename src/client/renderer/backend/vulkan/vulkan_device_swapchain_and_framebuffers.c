#include "vulkan_backend.h"

uint32_t vulkan_device_swapchain_and_framebuffers_create(struct game_client* game) {

	VkSurfaceCapabilitiesKHR surface_capabilities = { 0 };
	VKCall(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(game->renderer_state.backend.gpu, game->renderer_state.backend.surface, &surface_capabilities));

	game->renderer_state.backend.swapchain_image_count = surface_capabilities.minImageCount +
		(surface_capabilities.minImageCount == surface_capabilities.maxImageCount ? 0 : 1);
	
	VkSwapchainCreateInfoKHR swapchain_info = { 0 };
	swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_info.surface = game->renderer_state.backend.surface;
	swapchain_info.minImageCount = game->renderer_state.backend.swapchain_image_count;
	swapchain_info.imageFormat = game->renderer_state.backend.surface_format.format;
	swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_info.preTransform = surface_capabilities.currentTransform;
	swapchain_info.imageExtent = surface_capabilities.currentExtent;
	swapchain_info.imageArrayLayers = 1;
	swapchain_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;

	VKCall(vkCreateSwapchainKHR(game->renderer_state.backend.device, &swapchain_info, 0, &game->renderer_state.backend.swapchain));

	VKCall(vkGetSwapchainImagesKHR(game->renderer_state.backend.device, game->renderer_state.backend.swapchain, &game->renderer_state.backend.swapchain_image_count, game->renderer_state.backend.swapchain_images));

	VkImageViewCreateInfo image_view_info = { 0 };
	image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_info.format = game->renderer_state.backend.surface_format.format;
	image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	image_view_info.subresourceRange.layerCount = 1;
	image_view_info.subresourceRange.levelCount = 1;

	for (uint32_t i = 0; i < game->renderer_state.backend.swapchain_image_count; i++) {

		image_view_info.image = game->renderer_state.backend.swapchain_images[i];
		VKCall(vkCreateImageView(game->renderer_state.backend.device, &image_view_info, 0, &game->renderer_state.backend.swapchain_image_views[i]));
	}

	VkFramebufferCreateInfo framebuffer_info = { 0 };
	framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_info.width = game->application_state.main_window.width;
	framebuffer_info.height = game->application_state.main_window.height;
	framebuffer_info.renderPass = game->renderer_state.backend.window_render_pass;
	framebuffer_info.layers = 1;
	framebuffer_info.attachmentCount = 1;

	for (uint32_t i = 0; i < game->renderer_state.backend.swapchain_image_count; i++) {
		framebuffer_info.pAttachments = &game->renderer_state.backend.swapchain_image_views[i];
		VKCall(vkCreateFramebuffer(game->renderer_state.backend.device, &framebuffer_info, 0, &game->renderer_state.backend.framebuffers[i]));
	}


	return 0;
}

uint32_t vulkan_device_swapchain_and_framebuffers_destroy(struct game_client* game) {

	vkDeviceWaitIdle(game->renderer_state.backend.device);

	for (uint32_t i = 0; i < game->renderer_state.backend.swapchain_image_count; i++) {
		vkDestroyFramebuffer(game->renderer_state.backend.device, game->renderer_state.backend.framebuffers[i], 0);
		vkDestroyImageView(game->renderer_state.backend.device, game->renderer_state.backend.swapchain_image_views[i], 0);
	}
	vkDestroySwapchainKHR(game->renderer_state.backend.device, game->renderer_state.backend.swapchain, 0);

	return 0;
}