#include "testing.h"
#include <stdio.h>
#include <math.h>

#include "general/platformlib/platform/platform.h"
#include "client/rendering/rendering_context.h"

#include <vulkan/vulkan.h>
#include <malloc.h>

#include "test/pixel_char.h"

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
	return 0;
}

int main(int argc, char* argv[]) {

	platform_init();
	show_console_window();

	uint32_t apiVersion;
	if (vkEnumerateInstanceVersion(&apiVersion) == VK_SUCCESS) {
		printf("Running Vulkan %d.%d.%d\n", VK_API_VERSION_MAJOR(apiVersion), VK_API_VERSION_MINOR(apiVersion), VK_API_VERSION_PATCH(apiVersion));
	}

	uint32_t window = window_create(10, 10, 400, 400, "window");

	screen_size.width = window_get_width(window);
	screen_size.height = window_get_height(window);

	VKCall(new_VkInstance("testapp", "testengine", &instance, &debug_messenger));

	VKCall(create_vulkan_surface(instance, window, &surface));

	uint32_t queue_idx;

	VKCall(new_VkDevice(instance, surface, &gpu, &queue_idx, 1.f, &device));

	vkGetDeviceQueue(device, queue_idx, 0, &graphics_queue);

	VkSurfaceCapabilitiesKHR surface_capabilities = { 0 };
	VKCall(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surface_capabilities));

	sc_image_count = surface_capabilities.minImageCount + 1;
	if (surface_capabilities.minImageCount == surface_capabilities.maxImageCount) sc_image_count = surface_capabilities.minImageCount;

	VkSurfaceFormatKHR surface_format;
	VKCall(new_VkSwapchainKHR(device, gpu, surface, &sc_image_count, &swapchain, &surface_format, sc_images, sc_image_views));

	VkAttachmentDescription color_attachment = { 0 };
	color_attachment.format = surface_format.format;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
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

	VkDescriptorSetLayoutBinding pixel_char_buffer_binding = { 0 };
	pixel_char_buffer_binding.binding = 0;
	pixel_char_buffer_binding.descriptorCount = 1;
	pixel_char_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	pixel_char_buffer_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding pixel_font_buffer_binding = { 0 };
	pixel_font_buffer_binding.binding = 1;
	pixel_font_buffer_binding.descriptorCount = 1;
	pixel_font_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	pixel_font_buffer_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding bindings[] = {
		pixel_char_buffer_binding,
		pixel_font_buffer_binding,
	};

	VkDescriptorSetLayoutCreateInfo set_layout_info = { 0 };
	set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set_layout_info.bindingCount = 2;
	set_layout_info.pBindings = bindings;

	VKCall(vkCreateDescriptorSetLayout(device, &set_layout_info, 0, &set_layout));


	VkPipelineLayoutCreateInfo pipeline_layout_info = { 0 };
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &set_layout;

	VKCall(vkCreatePipelineLayout(device, &pipeline_layout_info, 0, &pipe_layout));


	VkPipelineVertexInputStateCreateInfo vertex_input_stage = { 0 };
	vertex_input_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkPipelineColorBlendAttachmentState color_blend_attachment = { 0 };
	color_blend_attachment.blendEnable = VK_TRUE;
	color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo color_blend_state = { 0 };
	color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_state.logicOpEnable = VK_FALSE;
	color_blend_state.logicOp = VK_LOGIC_OP_COPY;
	color_blend_state.attachmentCount = 1;
	color_blend_state.pAttachments = &color_blend_attachment;
	color_blend_state.blendConstants[0] = 0.0f;
	color_blend_state.blendConstants[1] = 0.0f;
	color_blend_state.blendConstants[2] = 0.0f;
	color_blend_state.blendConstants[3] = 0.0f;

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

	VKCall(new_VkShaderModule(device, "../../../resources/vertex_shader.spv", &vertex_shader));
	VKCall(new_VkShaderModule(device, "../../../resources/fragment_shader.spv", &fragment_shader));

	VkPipelineShaderStageCreateInfo shader_stages[] = {
		shader_stage(vertex_shader, VK_SHADER_STAGE_VERTEX_BIT),
		shader_stage(fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT)
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
	pipe_info.pStages = shader_stages;
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

	struct rendering_buffer pixel_char_buffer;
	struct rendering_buffer pixel_font_buffer;

	struct rendering_memory_manager rmm;
	VKCall(rendering_memory_manager_new(device, gpu, graphics_queue, command_pool, &rmm));

	VKCall(VkBuffer_new(&rmm, 65536, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &pixel_char_buffer));
	VKCall(VkBuffer_new(&rmm, sizeof(struct pixel_font), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &pixel_font_buffer));

	struct pixel_font* font = load_pixel_font("../../../resources/client/assets/fonts/debug.pixelfont");

	VKCall(VkBuffer_fill(&rmm, &pixel_font_buffer, font, sizeof(struct pixel_font)));

	VkSamplerCreateInfo sampler_info = { 0 };
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.minFilter = VK_FILTER_NEAREST;
	sampler_info.magFilter = VK_FILTER_NEAREST;

	VKCall(vkCreateSampler(device, &sampler_info, 0, &sampler));


	VkDescriptorPoolSize pool_sizes[2];

	pool_sizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pool_sizes[0].descriptorCount = 1;
	pool_sizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	pool_sizes[1].descriptorCount = 2;

	VkDescriptorPoolCreateInfo descriptor_pool_info = { 0 };
	descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_pool_info.maxSets = 1;
	descriptor_pool_info.poolSizeCount = 2;
	descriptor_pool_info.pPoolSizes = pool_sizes;

	VKCall(vkCreateDescriptorPool(device, &descriptor_pool_info, 0, &descriptor_pool));


	VkDescriptorSetAllocateInfo descriptor_set_info = { 0 };
	descriptor_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptor_set_info.pSetLayouts = &set_layout;
	descriptor_set_info.descriptorSetCount = 1;
	descriptor_set_info.descriptorPool = descriptor_pool;

	VKCall(vkAllocateDescriptorSets(device, &descriptor_set_info, &descriptor_set));

	VkDescriptorBufferInfo pixel_char_buffer_info = { 0 };
	pixel_char_buffer_info.buffer = pixel_char_buffer.buffer;
	pixel_char_buffer_info.range = VK_WHOLE_SIZE;

	VkDescriptorBufferInfo pixel_font_buffer_info = { 0 };
	pixel_font_buffer_info.buffer = pixel_font_buffer.buffer;
	pixel_font_buffer_info.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet pixel_char_buffer_write = { 0 };
	pixel_char_buffer_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	pixel_char_buffer_write.dstSet = descriptor_set;
	pixel_char_buffer_write.pBufferInfo = &pixel_char_buffer_info;
	pixel_char_buffer_write.dstBinding = 0;
	pixel_char_buffer_write.descriptorCount = 1;
	pixel_char_buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

	VkWriteDescriptorSet pixel_font_buffer_write = { 0 };
	pixel_font_buffer_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	pixel_font_buffer_write.dstSet = descriptor_set;
	pixel_font_buffer_write.pBufferInfo = &pixel_font_buffer_info;
	pixel_font_buffer_write.dstBinding = 1;
	pixel_font_buffer_write.descriptorCount = 1;
	pixel_font_buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

	VkWriteDescriptorSet descriptor_writes[] = { pixel_char_buffer_write, pixel_font_buffer_write };

	vkUpdateDescriptorSets(device, 2, descriptor_writes, 0, 0);

#define FRAME_TIME_FRAMES_AVERAGE 128

#define FPS 60.
	double last_frame_times[FRAME_TIME_FRAMES_AVERAGE] = { 0 };
	for (int32_t i = 0; i < FRAME_TIME_FRAMES_AVERAGE; i++) last_frame_times[i] = 1000. / FPS;

	int32_t render = 1;

	while (!get_key_state(KEY_ESCAPE)) {

		struct window_event event;
		while (window_process_next_event(&event)) if(event.type == WINDOW_EVENT_DESTROY) goto close;

		uint32_t new_width = window_get_width(window);
		uint32_t new_height = window_get_height(window);

		if (new_width != 0 && new_height != 0) {

			if (screen_size.width != new_width || screen_size.height != new_height) {

				screen_size.width = new_width;
				screen_size.height = new_height;

				vkDeviceWaitIdle(device);

				for (uint32_t i = 0; i < sc_image_count; i++) {
					vkDestroyImageView(device, sc_image_views[i], 0);
					vkDestroyFramebuffer(device, framebuffers[i], 0);
				}
				vkDestroySwapchainKHR(device, swapchain, 0);

				new_VkSwapchainKHR(device, gpu, surface, &sc_image_count, &swapchain, &surface_format, sc_images, sc_image_views);

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

			}

			render = 1;
		}
		else render = 0;

		double start_time = get_time();

		if (render) {

			struct character {
				uint64_t size;
				float start_position[2];
				struct pixel_char pixel_char_data;
			};

			void* pixel_char_data_host_buffer = malloc(sizeof(float) * 4 + sizeof(struct character) * 3);

			((float*)pixel_char_data_host_buffer)[0] = screen_size.width;
			((float*)pixel_char_data_host_buffer)[1] = screen_size.height;

			struct character* chars = (size_t)pixel_char_data_host_buffer + sizeof(float) * 4;

			chars[0] = (struct character){ 10, {100.f, 100.f}, { { 0.f, 1.f, 1.f, 1.f }, { 0.f, 0.f, 0.f, 1.f }, 'A', PIXEL_CHAR_UNDERLINE_MASK | PIXEL_CHAR_CURSIVE_MASK} };
			chars[1] = (struct character){ 10, {200.f, 150.f}, { { 1.f, 1.f, 0.f, 1.f }, { 0.f, 0.f, 0.f, 1.f }, '>', 0 } };
			chars[2] = (struct character){ 10, {300.f, 200.f}, { { 1.f, 0.f, 1.f, 1.f }, { 0.f, 0.f, 0.f, 1.f }, 'C', 0 } };

			VkBuffer_fill(&rmm, &pixel_char_buffer, pixel_char_data_host_buffer, sizeof(float) * 2 + sizeof(struct character) * 3);

			free(pixel_char_data_host_buffer);

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
			clear_value.color = (VkClearColorValue){ 0.5f, 0.5f, 0.5f, 1 };

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
			vkCmdDraw(cmd, 18, 1, 0, 0);


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

		}

		double average = 0;

		for (int32_t i = FRAME_TIME_FRAMES_AVERAGE - 2; i >= 0; i--) {
			last_frame_times[i + 1] = last_frame_times[i];
			average += last_frame_times[i];
		}


		last_frame_times[0] = get_time() - start_time;

		if ((int32_t)(1000. / FPS - last_frame_times[0]) > 1) {
			sleep_for_ms((uint32_t)(1000. / FPS - last_frame_times[0]));
			last_frame_times[0] = get_time() - start_time;
		}

		average = (average + last_frame_times[0]) / FRAME_TIME_FRAMES_AVERAGE;

		printf("               \033[0G");
		printf("FPS %f\033[0G", 1000. / average);
		
	}
	

close:

	VKCall(rendering_memory_manager_destroy(&rmm));

	window_destroy(window);

	platform_exit();

	return 0;
}