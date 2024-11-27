#include "rendering_window.h"

#include "general/platformlib/platform/platform.h"
#include "rendering_context.h"

int32_t rendering_window_new(struct rendering_window* window, struct rendering_core* core, uint32_t posx, uint32_t posy, uint32_t width, uint32_t height, uint8_t* name) {

	window->window = window_create(posx, posy, width, height, name);

	window->width = window_get_width(window->window);
	window->height = window_get_height(window->window);

	VKCall(create_vulkan_surface(core->instance, window, &window->surface));

	VKCall(new_VkDevice(core->instance, window->surface, &window->gpu, &window->queue_index, 1.f, &window->device));

	vkGetDeviceQueue(window->device, window->queue_index, 0, &window->graphics_queue);

	VkSurfaceCapabilitiesKHR surface_capabilities = { 0 };
	VKCall(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(window->gpu, window->surface, &surface_capabilities));

	window->sc_image_count = surface_capabilities.minImageCount + 1;
	if (surface_capabilities.minImageCount == surface_capabilities.maxImageCount) window->sc_image_count = surface_capabilities.minImageCount;

	VKCall(new_VkSwapchainKHR(window->device, window->gpu, window->surface, &window->sc_image_count, &window->swapchain, &window->surface_format, window->sc_images, window->sc_image_views));

	return 0;
}

int32_t game_window_resize(struct rendering_window* window, uint32_t new_width, uint32_t new_height) {

	window->width = new_width;
	window->height = new_height;

	vkDeviceWaitIdle(window->device);

	for (uint32_t i = 0; i < window->sc_image_count; i++) {
		vkDestroyImageView(window->device, window->sc_image_views[i], 0);
	}
	vkDestroySwapchainKHR(window->device, window->swapchain, 0);

	VKCall(new_VkSwapchainKHR(window->device, window->gpu, window->surface, &window->sc_image_count, &window->swapchain, &window->surface_format, window->sc_images, window->sc_image_views));

	return 0;
}