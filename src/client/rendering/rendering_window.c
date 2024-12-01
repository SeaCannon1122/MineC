#include "rendering_window.h"

#include "general/platformlib/platform/platform.h"
#include "rendering_context.h"

int32_t rendering_window_new(struct rendering_window* window, VkInstance instance, uint32_t posx, uint32_t posy, uint32_t width, uint32_t height, uint8_t* name) {

	window->window = window_create(posx, posy, width, height, name, 1);

	window->width = window_get_width(window->window);
	window->height = window_get_height(window->window);

	VKCall(create_vulkan_surface(instance, window->window, &window->surface));

	return 0;
}

int32_t rendering_window_swapchain_create(struct rendering_window* window, VkPhysicalDevice gpu, VkDevice device, VkSurfaceFormatKHR surface_format, VkRenderPass render_pass) {

	window->gpu = gpu;
	window->device = device;
	window->surface_format = surface_format;
	window->render_pass = render_pass;

	VkSurfaceCapabilitiesKHR surface_capabilities = { 0 };
	VKCall(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(window->gpu, window->surface, &surface_capabilities));

	window->sc_image_count = surface_capabilities.minImageCount + 1;
	if (surface_capabilities.minImageCount == surface_capabilities.maxImageCount) window->sc_image_count = surface_capabilities.minImageCount;

	VkSwapchainCreateInfoKHR sc_info = { 0 };
	sc_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	sc_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	sc_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	sc_info.surface = window->surface;
	sc_info.imageFormat = surface_format.format;
	sc_info.preTransform = surface_capabilities.currentTransform;
	sc_info.imageExtent = surface_capabilities.currentExtent;
	sc_info.minImageCount = window->sc_image_count;
	sc_info.imageArrayLayers = 1;

	VKCall(vkCreateSwapchainKHR(window->device, &sc_info, 0, &window->swapchain));

	VKCall(vkGetSwapchainImagesKHR(window->device, window->swapchain, &window->sc_image_count, window->sc_images));

	VkImageViewCreateInfo image_view_info = { 0 };
	image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_info.format = surface_format.format;
	image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	image_view_info.subresourceRange.layerCount = 1;
	image_view_info.subresourceRange.levelCount = 1;

	for (uint32_t i = 0; i < window->sc_image_count; i++) {

		image_view_info.image = window->sc_images[i];
		VKCall(vkCreateImageView(window->device, &image_view_info, 0, &window->sc_image_views[i]));
	}

	VkFramebufferCreateInfo framebuffer_info = { 0 };
	framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_info.width = window->width;
	framebuffer_info.height = window->height;
	framebuffer_info.renderPass = window->render_pass;
	framebuffer_info.layers = 1;
	framebuffer_info.attachmentCount = 1;

	for (uint32_t i = 0; i < window->sc_image_count; i++) {
		framebuffer_info.pAttachments = &window->sc_image_views[i];
		VKCall(vkCreateFramebuffer(window->device, &framebuffer_info, 0, &window->framebuffers[i]));
	}

	return 0;
}

int32_t rendering_window_swapchain_destroy(struct rendering_window* window) {

	vkDeviceWaitIdle(window->device);

	for (uint32_t i = 0; i < window->sc_image_count; i++) {
		vkDestroyFramebuffer(window->device, window->framebuffers[i], 0);
		vkDestroyImageView(window->device, window->sc_image_views[i], 0);
	}
	vkDestroySwapchainKHR(window->device, window->swapchain, 0);

	return 0;
}

int32_t rendering_window_resize(struct rendering_window* window, uint32_t new_width, uint32_t new_height) {

	window->width = new_width;
	window->height = new_height;

	rendering_window_swapchain_destroy(window);
	rendering_window_swapchain_create(window, window->gpu, window->device, window->surface_format, window->render_pass);

	return 0;
}