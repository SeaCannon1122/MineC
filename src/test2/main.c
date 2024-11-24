#include "testing.h"
#include <stdio.h>

#include "general/platformlib/networking.h"
#include "general/platformlib/platform/platform.h"
#include "client/rendering/rendering_context.h"

#include <vulkan/vulkan.h>
#include <malloc.h>

VkExtent2D screen_size;

VkInstance instance;
VkDebugUtilsMessengerEXT debug_messenger;
VkSurfaceKHR surface;
VkPhysicalDevice gpu;
VkDevice device;
VkQueue graphics_queue;
VkSwapchainKHR swapchain;
VkImage sc_images[5];
uint32_t sc_image_count;
VkFramebuffer framebuffers[5];
VkImageView sc_image_views[5];

VkCommandPool command_pool;
VkCommandBuffer cmd;

VkSemaphore submit_semaphore;
VkSemaphore aquire_semaphore;
VkFence img_available_fence;

VkDescriptorSetLayout set_layout;
VkPipeline pipeline;
VkPipelineLayout pipe_layout;
VkDescriptorSet descriptor_set;
VkDescriptorPool descriptor_pool;
VkSampler sampler;


VkRenderPass render_pass;

#define VKCall(call) \
do { \
    VkResult result = (call); \
    if (result != VK_SUCCESS) { \
        printf("Vulkan error in \n    %s \n at %s:%d: %d\n", #call, __FILE__, __LINE__, result); \
		DEBUG_BREAK();\
    } \
} while(0)

static void* load_file(char* filename, int* size) {

	FILE* file = fopen(filename, "rb");
	if (file == NULL) return NULL;

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	rewind(file);

	char* buffer = (char*)malloc(fileSize);
	if (buffer == NULL) {
		fclose(file);
		return NULL;
	}

	size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);

	fclose(file);

	*size = fileSize;

	return buffer;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callbck(
	VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
	VkDebugUtilsMessageTypeFlagsEXT msg_flags,
	const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
	void* p_user_data
) {
	printf("--------------------------------------------------------------------------\n\nValidation Error: %s\n\n\n\n", p_callback_data->pMessage);
	return false;
}

int main(int argc, char* argv[]) {

	platform_init();
	show_console_window();

	uint32_t window = window_create(10, 10, 1000, 1000, "window");

	screen_size.width = window_get_width(window);
	screen_size.height = window_get_height(window);

	// (semi) Vulkan code

	VKCall(new_VkInstance("testapp", "testengine", &instance, &debug_messenger));

	VKCall(create_vulkan_surface(instance, window, &surface));

	uint32_t queue_idx = -1;

	VKCall(get_first_suitable_VkPhysicalDevice(instance, surface, &gpu, &queue_idx));

	if (queue_idx == -1) {
		printf("couldnt fint suitable device");
		goto close;
	}

	VKCall(new_VkDevice(instance, gpu, queue_idx, 1.0f, &device));

	vkGetDeviceQueue(device, queue_idx, 0, &graphics_queue);

	VkSurfaceFormatKHR surface_format;
	VKCall(get_first_suitable_VkSurfaceFormatKHR(gpu, surface, &surface_format));

	VKCall(new_SwapchainKHR(device, gpu, surface, surface_format, &swapchain));

	VKCall(vkGetSwapchainImagesKHR(device, swapchain, &sc_image_count, 0));
	VKCall(vkGetSwapchainImagesKHR(device, swapchain, &sc_image_count, sc_images));

	VkImageViewCreateInfo image_view_info = { 0 };
	image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_info.format = surface_format.format;
	image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	image_view_info.subresourceRange.layerCount = 1;
	image_view_info.subresourceRange.levelCount = 1;

	for (uint32_t i = 0; i < sc_image_count; i++) {

		image_view_info.image = sc_images[i];
		VKCall(vkCreateImageView(device, &image_view_info, 0, &sc_image_views[i]));
	}



	VkAttachmentDescription color_attachment = { 0 };
	color_attachment.format = surface_format.format;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
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

	VKCall(vkCreateRenderPass(device, &render_pass_info, 0, &render_pass));

	VkFramebufferCreateInfo framebuffer_info = { 0 };
	framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_info.width = screen_size.width;
	framebuffer_info.height = screen_size.height;
	framebuffer_info.renderPass = render_pass;
	framebuffer_info.layers = 1;
	framebuffer_info.attachmentCount = 1;

	for (uint32_t i = 0; i < sc_image_count; i++) {
		framebuffer_info.pAttachments = &sc_image_views[i];
		VKCall(vkCreateFramebuffer(device, &framebuffer_info, 0, &framebuffers[i]));
	}


	VkCommandPoolCreateInfo pool_info = { 0 };
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = queue_idx;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VKCall(vkCreateCommandPool(device, &pool_info, 0, &command_pool));

	VkCommandBufferAllocateInfo alloc_info = { 0 };
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandBufferCount = 1;
	alloc_info.commandPool = command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	VKCall(vkAllocateCommandBuffers(device, &alloc_info, &cmd));

	VkSemaphoreCreateInfo sema_info = { 0 };
	sema_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VKCall(vkCreateSemaphore(device, &sema_info, 0, &aquire_semaphore));
	VKCall(vkCreateSemaphore(device, &sema_info, 0, &submit_semaphore));

	VkFenceCreateInfo fence_info = { 0 };
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VKCall(vkCreateFence(device, &fence_info, 0, &img_available_fence));


	VkDescriptorSetLayoutBinding binding = { 0 };
	binding.binding = 0;
	binding.descriptorCount = 1;
	binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo set_layout_info = { 0 };
	set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set_layout_info.bindingCount = 1;
	set_layout_info.pBindings = &binding;

	VKCall(vkCreateDescriptorSetLayout(device, &set_layout_info, 0, &set_layout));


	VkPipelineLayoutCreateInfo pipeline_layout_info = { 0 };
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &set_layout;

	VKCall(vkCreatePipelineLayout(device, &pipeline_layout_info, 0, &pipe_layout));


	VkPipelineVertexInputStateCreateInfo vertex_input_stage = { 0 };
	vertex_input_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkPipelineColorBlendAttachmentState color_blend_attachment = { 0 };
	color_blend_attachment.blendEnable = VK_FALSE;
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;


	VkPipelineColorBlendStateCreateInfo color_blend_state = { 0 };
	color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_state.pAttachments = &color_blend_attachment;
	color_blend_state.attachmentCount = 1;

	VkPipelineInputAssemblyStateCreateInfo input_assembly = { 0 };
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkRect2D initial_scissor = { 0 };
	VkViewport initial_viewport = { 0 };
	initial_viewport.maxDepth = 1.0f;

	VkPipelineViewportStateCreateInfo viewport_state = { 0 };
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = &initial_scissor;
	viewport_state.viewportCount = 1;
	viewport_state.pViewports = &initial_viewport;

	VkPipelineRasterizationStateCreateInfo rasterization_state = { 0 };
	rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_state.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterization_state.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterization_state.polygonMode = VK_POLYGON_MODE_FILL;
	rasterization_state.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo multi_sample_state = { 0 };
	multi_sample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multi_sample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkShaderModule vertex_shader, fragment_shader;

	uint32_t vertex_shader_code_size; 
	char* vertex_shader_code = load_file("../../../resources/vertex_shader.spv", &vertex_shader_code_size);

	uint32_t fragment_shader_code_size;
	char* fragment_shader_code = load_file("../../../resources/fragment_shader.spv", &fragment_shader_code_size);

	VkShaderModuleCreateInfo shader_info = { 0 };
	shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;


	shader_info.pCode = vertex_shader_code;
	shader_info.codeSize = vertex_shader_code_size;
	VKCall(vkCreateShaderModule(device, &shader_info, 0, &vertex_shader));

	shader_info.pCode = fragment_shader_code;
	shader_info.codeSize = fragment_shader_code_size;
	VKCall(vkCreateShaderModule(device, &shader_info, 0, &fragment_shader));

	free(vertex_shader_code);
	free(fragment_shader_code);

	VkPipelineShaderStageCreateInfo vertex_stage = { 0 };
	vertex_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertex_stage.pName = "main";
	vertex_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertex_stage.module = vertex_shader;

	VkPipelineShaderStageCreateInfo fragment_stage = { 0 };
	fragment_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragment_stage.pName = "main";
	fragment_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragment_stage.module = fragment_shader;

	VkPipelineShaderStageCreateInfo shader_stage[] = {
		vertex_stage,
		fragment_stage
	};

	VkDynamicState dynamic_states[] = {
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_VIEWPORT
	};

	VkPipelineDynamicStateCreateInfo dynamic_state = { 0 };
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.pDynamicStates = dynamic_states;
	dynamic_state.dynamicStateCount = 2;

	VkGraphicsPipelineCreateInfo pipe_info = { 0 };
	pipe_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipe_info.pVertexInputState = &vertex_input_stage;
	pipe_info.pColorBlendState = &color_blend_state;
	pipe_info.pStages = shader_stage;
	pipe_info.layout = pipe_layout;
	pipe_info.renderPass = render_pass;
	pipe_info.stageCount = 2;
	pipe_info.pRasterizationState = &rasterization_state;
	pipe_info.pViewportState = &viewport_state;
	pipe_info.pDynamicState = &dynamic_state;
	pipe_info.pMultisampleState = &multi_sample_state;
	pipe_info.pInputAssemblyState = &input_assembly;

	VKCall(vkCreateGraphicsPipelines(device, 0, 1, &pipe_info, 0, &pipeline));

	vkDestroyShaderModule(device, vertex_shader, 0);
	vkDestroyShaderModule(device, fragment_shader, 0);

	struct rendering_image image;
	struct rendering_buffer staging_buffer;

	VKCall(new_VkBuffer(device, gpu, 512 * 512 * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &staging_buffer));
	VKCall(vkMapMemory(device, staging_buffer.memory, 0, 512 * 512 * 4, 0, &staging_buffer.data));
	VKCall(vkBindBufferMemory(device, staging_buffer.buffer, staging_buffer.memory, 0));

	VKCall(new_VkImage(device, gpu, graphics_queue, command_pool, "C:/Users/coroc/Desktop/resources/oak_leaves.png", &staging_buffer, &image));

	VkSamplerCreateInfo sampler_info = { 0 };
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.minFilter = VK_FILTER_NEAREST;
	sampler_info.magFilter = VK_FILTER_NEAREST;

	VKCall(vkCreateSampler(device, &sampler_info, 0, &sampler));


	VkDescriptorPoolSize pool_size = { 0 };
	pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pool_size.descriptorCount = 1;

	VkDescriptorPoolCreateInfo descriptor_pool_info = { 0 };
	descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_pool_info.maxSets = 1;
	descriptor_pool_info.poolSizeCount = 1;
	descriptor_pool_info.pPoolSizes = &pool_size;

	VKCall(vkCreateDescriptorPool(device, &descriptor_pool_info, 0, &descriptor_pool));


	VkDescriptorSetAllocateInfo descriptor_set_info = { 0 };
	descriptor_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptor_set_info.pSetLayouts = &set_layout;
	descriptor_set_info.descriptorSetCount = 1;
	descriptor_set_info.descriptorPool = descriptor_pool;

	VKCall(vkAllocateDescriptorSets(device, &descriptor_set_info, &descriptor_set));

	VkDescriptorImageInfo img_info = { 0 };
	img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	img_info.imageView = image.view;
	img_info.sampler = sampler;

	VkWriteDescriptorSet write = { 0 };
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = descriptor_set;
	write.pImageInfo = &img_info;
	write.dstBinding = 0;
	write.descriptorCount = 1;
	write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

	vkUpdateDescriptorSets(device, 1, &write, 0, 0);


	while (!get_key_state(KEY_ESCAPE)) {

		struct window_event event;

		while (window_process_next_event(&event)) {

			switch (event.type) {

			case WINDOW_EVENT_SIZE: {

				screen_size.width = event.info.window_event_size.width;
				screen_size.height = event.info.window_event_size.height;

				printf("resize to %d %d\n", event.info.window_event_size.width, event.info.window_event_size.height);
			} break;

			case WINDOW_EVENT_MOVE: {
				printf("moved to %d %d\n", event.info.window_event_move.x_position, event.info.window_event_move.y_position);
			}

			case WINDOW_EVENT_DESTROY: {
				goto close;
			}

			}


		}

		uint32_t img_index;
		
		VKCall(vkWaitForFences(device, 1, &img_available_fence, VK_TRUE, UINT64_MAX));
		VKCall(vkResetFences(device, 1, &img_available_fence));
		
		VKCall(vkAcquireNextImageKHR(device, swapchain, 0, aquire_semaphore, 0, &img_index));

		vkResetCommandBuffer(cmd, 0);

		VkCommandBufferBeginInfo begin_info = { 0 };
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VKCall(vkBeginCommandBuffer(cmd, &begin_info));

		VkClearValue clear_value = { 0 };
		clear_value.color = (VkClearColorValue){ 1, 0, 0, 1 };

		VkRenderPassBeginInfo renderpass_begin_info = { 0 };
		renderpass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpass_begin_info.renderPass = render_pass;
		renderpass_begin_info.renderArea.extent = screen_size;
		renderpass_begin_info.framebuffer = framebuffers[img_index];
		renderpass_begin_info.pClearValues = &clear_value;
		renderpass_begin_info.clearValueCount = 1;

		vkCmdBeginRenderPass(cmd, &renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

		VkRect2D scissor = { 0 };
		scissor.extent = screen_size;

		VkViewport viewport = { 0 };
		viewport.width = screen_size.width;
		viewport.height = screen_size.height;
		viewport.maxDepth = 1.0f;

		vkCmdSetViewport(cmd, 0, 1, &viewport);
		vkCmdSetScissor(cmd, 0, 1, &scissor);

		vkCmdBindDescriptorSets(
			cmd,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipe_layout,
			0,
			1,
			&descriptor_set,
			0,
			0
		);

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdDraw(cmd, 6, 1, 0, 0);


		vkCmdEndRenderPass(cmd);

		VKCall(vkEndCommandBuffer(cmd));

		VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo submit_info = { 0 };
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pWaitDstStageMask = &wait_stage;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &cmd;
		submit_info.pSignalSemaphores = &submit_semaphore;
		submit_info.pWaitSemaphores = &aquire_semaphore;
		submit_info.signalSemaphoreCount = 1;
		submit_info.waitSemaphoreCount = 1;


		VKCall(vkQueueSubmit(graphics_queue, 1, &submit_info, img_available_fence));

		VkPresentInfoKHR present_info = { 0 };
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.pSwapchains = &swapchain;
		present_info.swapchainCount = 1;
		present_info.pImageIndices = &img_index;
		present_info.pWaitSemaphores = &submit_semaphore;
		present_info.waitSemaphoreCount = 1;

		VKCall(vkQueuePresentKHR(graphics_queue, &present_info));

		sleep_for_ms(10);
	}
	

close:
	window_destroy(window);

	platform_exit();

	return 0;
}