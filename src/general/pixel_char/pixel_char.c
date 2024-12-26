#include "pixel_char.h"

#include "pixel_char_vertex_shader.h"
#include "pixel_char_fragment_shader.h"

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#if defined(_WIN32)

#define DEBUG_BREAK() __debugbreak()


#elif defined(__linux__)
#include <signal.h>
#define DEBUG_BREAK() raise(SIGTRAP)

#elif defined(__APPLE__)

#define DEBUG_BREAK __builtin_trap()
#define RESTRICT restrict

#endif

#define VKCall(call) \
do { \
    VkResult result = (call); \
    if (result != VK_SUCCESS) { \
        printf("Vulkan error in \n    %s \n at %s:%d: %d\n", #call, __FILE__, __LINE__, result); \
		DEBUG_BREAK();\
    } \
} while(0)

struct pixel_font* load_pixel_font(char* src) {

	FILE* file = fopen(src, "rb");
	if (file == NULL) return NULL;

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	rewind(file);

	void* buffer = calloc(1, sizeof(struct pixel_font));
	if (buffer == NULL) {
		fclose(file);
		return NULL;
	}

	size_t bytesRead = fread(buffer, 1, fileSize, file);

	fclose(file);

	return buffer;
}

uint32_t pixel_char_renderer_new(
	struct pixel_char_renderer* pcr, 
	VkDevice device, 
	VkPhysicalDevice gpu,
	VkRenderPass render_pass, 
	uint8_t* vertex_shader_custom,
	uint32_t vertex_shader_custom_length, 
	uint8_t* fragment_shader_custom, 
	uint32_t fragment_shader_custom_length
) {

	pcr->device = device;
	pcr->gpu = gpu;


	uint8_t* vertex_source = (vertex_shader_custom == 0 ? vertex_shader_default : vertex_shader_custom);
	uint32_t vertex_source_length = (vertex_shader_custom == 0 ? vertex_shader_default_length : vertex_shader_custom_length);

	uint8_t* fragment_source = (fragment_shader_custom == 0 ? fragment_shader_default : fragment_shader_custom);
	uint32_t fragment_source_length = (fragment_shader_custom == 0 ? fragment_shader_default_length : fragment_shader_custom_length);

	VkShaderModule vertex_shader, fragment_shader;

	VkShaderModuleCreateInfo shader_info = { 0 };
	shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

	shader_info.pCode = vertex_source;
	shader_info.codeSize = vertex_source_length;
	if (vkCreateShaderModule(pcr->device, &shader_info, 0, &vertex_shader) != VK_SUCCESS) {
		printf("[PIXELCHAR RENDERER] Couldn't create Vertex-ShaderModule\n");

		return 1;
	}

	shader_info.pCode = fragment_source;
	shader_info.codeSize = fragment_source_length;
	if (vkCreateShaderModule(pcr->device, &shader_info, 0, &fragment_shader) != VK_SUCCESS) {
		printf("[PIXELCHAR RENDERER] Couldn't create Fragment-ShaderModule\n");

		vkDestroyShaderModule(pcr->device, vertex_shader, 0);
		return 1;
	}


	VkDescriptorSetLayoutBinding pixel_font_buffer_binding = { 0 };
	pixel_font_buffer_binding.descriptorCount = 1;
	pixel_font_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	pixel_font_buffer_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding bindings[MAX_PIXEL_FONTS];
	for (int32_t i = 0; i < MAX_PIXEL_FONTS; i++) {
		pixel_font_buffer_binding.binding = i;
		bindings[i] = pixel_font_buffer_binding;
	}

	VkDescriptorSetLayoutCreateInfo set_layout_info = { 0 };
	set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set_layout_info.bindingCount = MAX_PIXEL_FONTS;
	set_layout_info.pBindings = bindings;

	VKCall(vkCreateDescriptorSetLayout(pcr->device, &set_layout_info, 0, &pcr->set_layout));

	VkPushConstantRange push_constant_range = { 0 };
	push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	push_constant_range.offset = 0;
	push_constant_range.size = sizeof(uint32_t) * 3;

	VkPipelineLayoutCreateInfo pipeline_layout_info = { 0 };
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &pcr->set_layout;
	pipeline_layout_info.pushConstantRangeCount = 1;
	pipeline_layout_info.pPushConstantRanges = &push_constant_range;

	VKCall(vkCreatePipelineLayout(pcr->device, &pipeline_layout_info, 0, &pcr->pipe_layout));

	VkVertexInputBindingDescription vertex_binding_description = { 0 };
	vertex_binding_description.binding = 0;
	vertex_binding_description.stride = sizeof(struct pixel_char);
	vertex_binding_description.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;


	VkVertexInputAttributeDescription vertex_attribute_descriptions[] = {
		
		// Color (4 x 8-bit)
		{
			.binding = 0,
			.location = 0,
			.format = VK_FORMAT_R8G8B8A8_UNORM,
			.offset = offsetof(struct pixel_char, color[0])
		},
		// Background color (4 x 8-bit)
		{
			.binding = 0,
			.location = 1,
			.format = VK_FORMAT_R8G8B8A8_UNORM,
			.offset = offsetof(struct pixel_char, background_color[0])
		},
		// Value (32-bit integer)
		{
			.binding = 0,
			.location = 2,
			.format = VK_FORMAT_R32_UINT,
			.offset = offsetof(struct pixel_char, value)
		},
		// Start position (2 x 16-bit integers)
		{
			.binding = 0,
			.location = 3,
			.format = VK_FORMAT_R16G16_SINT,
			.offset = offsetof(struct pixel_char, position[0])
		},
		// Masks (16-bit integer)
		{
			.binding = 0,
			.location = 4,
			.format = VK_FORMAT_R16_UINT,
			.offset = offsetof(struct pixel_char, masks)
		},
		// Size (16-bit integer)
		{
			.binding = 0,
			.location = 5,
			.format = VK_FORMAT_R16_SINT,
			.offset = offsetof(struct pixel_char, size)
		},
		
	};


	VkPipelineVertexInputStateCreateInfo vertex_input_stage = { 0 };
	vertex_input_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_stage.vertexBindingDescriptionCount = 1;
	vertex_input_stage.pVertexBindingDescriptions = &vertex_binding_description;
	vertex_input_stage.vertexAttributeDescriptionCount = 6;
	vertex_input_stage.pVertexAttributeDescriptions = vertex_attribute_descriptions;


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

	VkPipelineShaderStageCreateInfo vertex_shader_stage = { 0 };
	vertex_shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertex_shader_stage.pName = "main";
	vertex_shader_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertex_shader_stage.module = vertex_shader;

	VkPipelineShaderStageCreateInfo fragment_shader_stage = { 0 };
	fragment_shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragment_shader_stage.pName = "main";
	fragment_shader_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragment_shader_stage.module = fragment_shader;

	VkPipelineShaderStageCreateInfo shader_stages[] = {
		vertex_shader_stage,
		fragment_shader_stage
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
	pipe_info.layout = pcr->pipe_layout;
	pipe_info.renderPass = render_pass;
	pipe_info.stageCount = 2;
	pipe_info.pRasterizationState = &rasterization_state;
	pipe_info.pViewportState = &viewport_state;
	pipe_info.pDynamicState = &dynamic_state;
	pipe_info.pMultisampleState = &multi_sample_state;
	pipe_info.pInputAssemblyState = &input_assembly;

	VkResult pipeline_result = vkCreateGraphicsPipelines(pcr->device, 0, 1, &pipe_info, 0, &pcr->pipeline);

	vkDestroyShaderModule(pcr->device, vertex_shader, 0);
	vkDestroyShaderModule(pcr->device, fragment_shader, 0);

	if (pipeline_result != VK_SUCCESS) {
		printf("[PIXELCHAR RENDERER] Couldn't create pixelchar graphics pipeline\n");

		vkDestroyPipelineLayout(pcr->device, pcr->pipe_layout, 0);
		vkDestroyDescriptorSetLayout(pcr->device, pcr->set_layout, 0);

		return 1;
	}

	uint32_t pixel_char_buffer_size = 4096 * sizeof(struct pixel_char);

	VkBufferCreateInfo buffer_info = { 0 };
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	buffer_info.size = pixel_char_buffer_size;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VKCall(vkCreateBuffer(pcr->device, &buffer_info, 0, &pcr->pixel_char_buffer));

	VkMemoryRequirements memory_requirements;

	vkGetBufferMemoryRequirements(pcr->device, pcr->pixel_char_buffer, &memory_requirements);

	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(pcr->gpu, &memory_properties);

	VkMemoryAllocateInfo alloc_info = { 0 };
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {

		if ((memory_requirements.memoryTypeBits & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) == (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
			alloc_info.memoryTypeIndex = i;
			break;
		}
	}

	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = (pixel_char_buffer_size < memory_requirements.size ? memory_requirements.size : pixel_char_buffer_size);

	VKCall(vkAllocateMemory(pcr->device, &alloc_info, 0, &pcr->pixel_char_buffer_memory));
	VKCall(vkBindBufferMemory(pcr->device, pcr->pixel_char_buffer, pcr->pixel_char_buffer_memory, 0));
	VKCall(vkMapMemory(pcr->device, pcr->pixel_char_buffer_memory, 0, pixel_char_buffer_size, 0, &pcr->pixel_char_buffer_host_handle));

	VkDescriptorPoolSize pool_size = { 0 };
	pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	pool_size.descriptorCount = MAX_PIXEL_FONTS + 1;;

	VkDescriptorPoolCreateInfo descriptor_pool_info = { 0 };
	descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_pool_info.maxSets = 1;
	descriptor_pool_info.poolSizeCount = 1;
	descriptor_pool_info.pPoolSizes = &pool_size;

	VKCall(vkCreateDescriptorPool(pcr->device, &descriptor_pool_info, 0, &pcr->descriptor_pool));


	VkDescriptorSetAllocateInfo descriptor_set_info = { 0 };
	descriptor_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptor_set_info.pSetLayouts = &pcr->set_layout;
	descriptor_set_info.descriptorSetCount = 1;
	descriptor_set_info.descriptorPool = pcr->descriptor_pool;

	VKCall(vkAllocateDescriptorSets(pcr->device, &descriptor_set_info, &pcr->descriptor_set));

	return 0;
}

uint32_t pixel_char_renderer_destroy(struct pixel_char_renderer* pcr) {

	vkDeviceWaitIdle(pcr->device);

	vkDestroyDescriptorPool(pcr->device, pcr->descriptor_pool, 0);

	vkDestroyPipeline(pcr->device, pcr->pipeline, 0);
	vkDestroyPipelineLayout(pcr->device, pcr->pipe_layout, 0);
	vkDestroyDescriptorSetLayout(pcr->device, pcr->set_layout, 0);

	vkUnmapMemory(pcr->device, pcr->pixel_char_buffer_memory);
	vkFreeMemory(pcr->device, pcr->pixel_char_buffer_memory, 0);
	vkDestroyBuffer(pcr->device, pcr->pixel_char_buffer, 0);

	return 0;
}

uint32_t pixel_char_renderer_add_font(struct pixel_char_renderer* pcr, VkBuffer buffer, uint32_t offset, uint32_t font_index) {

	if (font_index >= MAX_PIXEL_FONTS) return 1;

	VkDescriptorBufferInfo pixel_font_buffer_info = { 0 };
	pixel_font_buffer_info.buffer = buffer;
	pixel_font_buffer_info.range = sizeof(struct pixel_font);
	pixel_font_buffer_info.offset = offset;

	VkWriteDescriptorSet pixel_font_buffer_write = { 0 };
	pixel_font_buffer_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	pixel_font_buffer_write.dstSet = pcr->descriptor_set;
	pixel_font_buffer_write.pBufferInfo = &pixel_font_buffer_info;
	pixel_font_buffer_write.dstBinding = font_index;
	pixel_font_buffer_write.descriptorCount = 1;
	pixel_font_buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

	vkUpdateDescriptorSets(pcr->device, 1, &pixel_font_buffer_write, 0, 0);

	return 0;
}

uint32_t pixel_char_renderer_fill_chars(struct pixel_char_renderer* pcr, struct pixel_char* chars, uint32_t chars_count) {
	pcr->chars_to_draw = chars_count;

	memcpy(pcr->pixel_char_buffer_host_handle, chars, sizeof(struct pixel_char)* chars_count);

	return 0;
}

uint32_t pixel_char_renderer_render(struct pixel_char_renderer* pcr, VkCommandBuffer cmd, VkExtent2D screen_size) {

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
		pcr->pipe_layout,
		0,
		1,
		&pcr->descriptor_set,
		0,
		0
	);

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pcr->pipeline);

	VkDeviceSize device_size = 0;
	vkCmdBindVertexBuffers(cmd, 0, 1, &pcr->pixel_char_buffer, &device_size);

	vkCmdPushConstants(
		cmd,
		pcr->pipe_layout,
		VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		0,
		sizeof(uint32_t) * 2,
		&screen_size
	);

	for (uint32_t char_render_mode = 0; char_render_mode < 3; char_render_mode++) {
		vkCmdPushConstants(
			cmd,
			pcr->pipe_layout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			sizeof(uint32_t) * 2,
			sizeof(uint32_t),
			&char_render_mode
		);

		vkCmdDraw(cmd, 6, pcr->chars_to_draw, 0, 0);
	}

	return 0;
}