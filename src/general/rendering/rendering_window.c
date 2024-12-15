#include "rendering_window.h"

#include "general/platformlib/platform/platform.h"
#include "vulkan_helpers.h"

struct rendering_window_state rws[MAX_WINDOW_COUNT];

int32_t rendering_window_create_rendering_pass(VkDevice device, VkSurfaceFormatKHR surface_format, VkRenderPass* render_pass) {

	VkAttachmentDescription color_attachment = { 0 };
	color_attachment.format = surface_format.format;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription attachments[] = {
		color_attachment
	};

	VkAttachmentReference color_attachment_reference = { 0 };
	color_attachment_reference.attachment = 0;
	color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass_description = { 0 };
	subpass_description.colorAttachmentCount = 1;
	subpass_description.pColorAttachments = &color_attachment_reference;

	VkRenderPassCreateInfo render_pass_info = { 0 };
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.pAttachments = attachments;
	render_pass_info.attachmentCount = 1;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass_description;

	VKCall(vkCreateRenderPass(device, &render_pass_info, 0, render_pass));

	return 0;
}

int32_t rendering_window_renderpass_begin_info(uint32_t window, VkRenderPassBeginInfo* renderpass_begin_info, VkRenderPass render_pass, VkSemaphore aquire_semaphore) {

	VKCall(vkAcquireNextImageKHR(rws[window].device, rws[window].swapchain, 0, aquire_semaphore, 0, &rws[window].sc_image_index));

	VkExtent2D screen_size;
	screen_size.width = rws[window].width;
	screen_size.height = rws[window].height;

	renderpass_begin_info->sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpass_begin_info->renderPass = render_pass;
	renderpass_begin_info->renderArea.extent = screen_size;
	renderpass_begin_info->framebuffer = rws[window].framebuffers[rws[window].sc_image_index];

	return 0;
}

int32_t rendering_window_present_image(uint32_t window, VkQueue queue, VkSemaphore* wait_semaphores, uint32_t semaphore_count) {

	VkPresentInfoKHR present_info = { 0 };
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.pSwapchains = &rws[window].swapchain;
	present_info.swapchainCount = 1;
	present_info.pImageIndices = &rws[window].sc_image_index;
	present_info.pWaitSemaphores = wait_semaphores;
	present_info.waitSemaphoreCount = semaphore_count;

	VKCall(vkQueuePresentKHR(queue, &present_info));

	return 0;
}

int32_t rendering_window_new(uint32_t window, VkInstance instance) {

	rws[window].window = window;

	rws[window].width = window_get_width(rws[window].window);
	rws[window].height = window_get_height(rws[window].window);

	rws[window].instance = instance;
	rws[window].swapchain = NULL;

	VKCall(create_vulkan_surface(rws[window].instance, rws[window].window, &rws[window].surface));

	return 0;
}

int32_t rendering_window_destroy(uint32_t window) {

	if (rws[window].swapchain) rendering_window_swapchain_destroy(window);

	VKCall(destroy_vulkan_surface(rws[window].instance, rws[window].surface));
	window_destroy(rws[window].window);

	return 0;
}

int32_t rendering_window_swapchain_create(uint32_t window, VkPhysicalDevice gpu, VkDevice device, VkSurfaceFormatKHR surface_format, VkPresentModeKHR present_mode, VkRenderPass render_pass) {

	rws[window].gpu = gpu;
	rws[window].device = device;
	rws[window].surface_format = surface_format;
	rws[window].render_pass = render_pass;
	rws[window].present_mode = present_mode;

	VkSurfaceCapabilitiesKHR surface_capabilities = { 0 };
	VKCall(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(rws[window].gpu, rws[window].surface, &surface_capabilities));

	rws[window].sc_image_count = surface_capabilities.minImageCount + 1;
	if (surface_capabilities.minImageCount == surface_capabilities.maxImageCount) rws[window].sc_image_count = surface_capabilities.minImageCount;

	VkSwapchainCreateInfoKHR sc_info = { 0 };
	sc_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	sc_info.surface = rws[window].surface;
	sc_info.minImageCount = rws[window].sc_image_count;
	sc_info.imageFormat = surface_format.format;
	sc_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	sc_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	sc_info.preTransform = surface_capabilities.currentTransform;
	sc_info.imageExtent = surface_capabilities.currentExtent;
	sc_info.imageArrayLayers = 1;
	sc_info.presentMode = present_mode;
	
	VKCall(vkCreateSwapchainKHR(rws[window].device, &sc_info, 0, &rws[window].swapchain));

	VKCall(vkGetSwapchainImagesKHR(rws[window].device, rws[window].swapchain, &rws[window].sc_image_count, rws[window].sc_images));

	VkImageViewCreateInfo image_view_info = { 0 };
	image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_info.format = surface_format.format;
	image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	image_view_info.subresourceRange.layerCount = 1;
	image_view_info.subresourceRange.levelCount = 1;

	for (uint32_t i = 0; i < rws[window].sc_image_count; i++) {

		image_view_info.image = rws[window].sc_images[i];
		VKCall(vkCreateImageView(rws[window].device, &image_view_info, 0, &rws[window].sc_image_views[i]));
	}

	VkFramebufferCreateInfo framebuffer_info = { 0 };
	framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_info.width = rws[window].width;
	framebuffer_info.height = rws[window].height;
	framebuffer_info.renderPass = rws[window].render_pass;
	framebuffer_info.layers = 1;
	framebuffer_info.attachmentCount = 1;

	for (uint32_t i = 0; i < rws[window].sc_image_count; i++) {
		framebuffer_info.pAttachments = &rws[window].sc_image_views[i];
		VKCall(vkCreateFramebuffer(rws[window].device, &framebuffer_info, 0, &rws[window].framebuffers[i]));
	}

	return 0;
}

int32_t rendering_window_swapchain_destroy(uint32_t window) {

	vkDeviceWaitIdle(rws[window].device);

	for (uint32_t i = 0; i < rws[window].sc_image_count; i++) {
		vkDestroyFramebuffer(rws[window].device, rws[window].framebuffers[i], 0);
		vkDestroyImageView(rws[window].device, rws[window].sc_image_views[i], 0);
	}
	vkDestroySwapchainKHR(rws[window].device, rws[window].swapchain, 0);

	rws[window].swapchain = NULL;

	return 0;
}

int32_t rendering_window_resize(uint32_t window) {

	rws[window].width = window_get_width(rws[window].window);
	rws[window].height = window_get_height(rws[window].window);

	rendering_window_swapchain_destroy(window);
	rendering_window_swapchain_create(window, rws[window].gpu, rws[window].device, rws[window].surface_format, rws[window].present_mode, rws[window].render_pass);

	return 0;
}

int32_t rendering_window_get_data(int32_t window, struct rendering_window_state* state) {
	memcpy(state, &rws[window], sizeof(struct rendering_window_state));

	return 0;
}