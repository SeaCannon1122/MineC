#include "vulkan_backend.h"

uint32_t vulkan_device_renderpasses_create(struct minec_client* game) {

	//window renderpass

	VkAttachmentDescription color_attachment = { 0 };
	color_attachment.format = game->renderer_state.backend.surface_format.format;
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

	VKCall(vkCreateRenderPass(game->renderer_state.backend.device, &render_pass_info, 0, &game->renderer_state.backend.window_render_pass));

	return 0;
}

uint32_t vulkan_device_renderpasses_destroy(struct minec_client* game) {

	vkDeviceWaitIdle(game->renderer_state.backend.device);

	vkDestroyRenderPass(game->renderer_state.backend.device, game->renderer_state.backend.window_render_pass, 0);

	return 0;
}