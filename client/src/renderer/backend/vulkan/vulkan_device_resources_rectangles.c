#include "vulkan_backend.h"

#include <stdio.h>

struct render_rectangle_vertex {

	int16_t x;
	int16_t y;
	float u;
	float v;
	int32_t image_index_or_color;
	int32_t sampler_index;
};

uint32_t vulkan_device_resources_rectangles_create(struct minec_client* game) {

	game->renderer_state.backend.rectangles_count = 0; 

	uint32_t rectangles_buffer_size = 256 * 6 * sizeof(struct render_rectangle_vertex);

	VkBufferCreateInfo buffer_info = { 0 };
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	buffer_info.size = rectangles_buffer_size;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VKCall(vkCreateBuffer(game->renderer_state.backend.device, &buffer_info, 0, &game->renderer_state.backend.rectangles_buffer));

	VkMemoryRequirements memory_requirements;

	vkGetBufferMemoryRequirements(game->renderer_state.backend.device, game->renderer_state.backend.rectangles_buffer, &memory_requirements);

	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(game->renderer_state.backend.gpu, &memory_properties);

	VkMemoryAllocateInfo alloc_info = { 0 };
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {

		if ((memory_requirements.memoryTypeBits & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) == (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
			alloc_info.memoryTypeIndex = i;
			break;
		}
	}

	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = memory_requirements.size;

	VKCall(vkAllocateMemory(game->renderer_state.backend.device, &alloc_info, 0, &game->renderer_state.backend.rectangles_buffer_memory));
	VKCall(vkBindBufferMemory(game->renderer_state.backend.device, game->renderer_state.backend.rectangles_buffer, game->renderer_state.backend.rectangles_buffer_memory, 0));
	VKCall(vkMapMemory(game->renderer_state.backend.device, game->renderer_state.backend.rectangles_buffer_memory, 0, memory_requirements.size, 0, &game->renderer_state.backend.rectangles_buffer_memory_host_handle));

	game->renderer_state.backend.rectangles_pipeline_usable_bool = 0;

	VkPushConstantRange push_constant_range = { 0 };
	push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	push_constant_range.offset = 0;
	push_constant_range.size = sizeof(uint32_t) * 2;

	VkPipelineLayoutCreateInfo pipeline_layout_info = { 0 };
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &game->renderer_state.backend.images_descriptor_set_layout;
	pipeline_layout_info.pushConstantRangeCount = 1;
	pipeline_layout_info.pPushConstantRanges = &push_constant_range;

	VKCall(vkCreatePipelineLayout(game->renderer_state.backend.device, &pipeline_layout_info, 0, &game->renderer_state.backend.rectangles_pipeline_layout));

	if (game->resource_state.shader_atlas[SHADER_VULKAN_RECTANGLES_VERTEX].data == NULL || game->resource_state.shader_atlas[SHADER_VULKAN_RECTANGLES_FRAGMENT].data == NULL) return 1;

	VkShaderModule vertex_shader, fragment_shader;

	VkShaderModuleCreateInfo shader_info = { 0 };
	shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

	shader_info.pCode = (uint32_t*)game->resource_state.shader_atlas[SHADER_VULKAN_RECTANGLES_VERTEX].data;
	shader_info.codeSize = game->resource_state.shader_atlas[SHADER_VULKAN_RECTANGLES_VERTEX].size;
	if (vkCreateShaderModule(game->renderer_state.backend.device, &shader_info, 0, &vertex_shader) != VK_SUCCESS) {
		printf("[RENDERER BACKEND] Couldn't create ShaderModule from token '%s'\n", resources_shader_tokens[SHADER_VULKAN_RECTANGLES_VERTEX]);

		return 1;
	}

	shader_info.pCode = (uint32_t*)game->resource_state.shader_atlas[SHADER_VULKAN_RECTANGLES_FRAGMENT].data;
	shader_info.codeSize = game->resource_state.shader_atlas[SHADER_VULKAN_RECTANGLES_FRAGMENT].size;
	if (vkCreateShaderModule(game->renderer_state.backend.device, &shader_info, 0, &fragment_shader) != VK_SUCCESS) {
		printf("[RENDERER BACKEND] Couldn't create ShaderModule from token '%s'\n", resources_shader_tokens[SHADER_VULKAN_RECTANGLES_FRAGMENT]);

		vkDestroyShaderModule(game->renderer_state.backend.device, vertex_shader, 0);

		return 1;
	}


	VkVertexInputBindingDescription vertex_binding_description = { 0 };
	vertex_binding_description.binding = 0;
	vertex_binding_description.stride = sizeof(struct render_rectangle_vertex);
	vertex_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription vertex_attribute_descriptions[] = {

		// vertex position
		{
			.binding = 0,
			.location = 0,
			.format = VK_FORMAT_R16G16_SINT,
			.offset = 0
		},
		// image uv
		{
			.binding = 0,
			.location = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = 2 * sizeof(int16_t)
		},
		// image index
		{
			.binding = 0,
			.location = 2,
			.format = VK_FORMAT_R32_SINT,
			.offset = 2 * sizeof(int16_t) + 2 * sizeof(float)
		},
		//sampler index
		{
			.binding = 0,
			.location = 3,
			.format = VK_FORMAT_R32_SINT,
			.offset = 2 * sizeof(int16_t) + 2 * sizeof(float) + sizeof(int32_t)
		},
	};


	VkPipelineVertexInputStateCreateInfo vertex_input_stage = { 0 };
	vertex_input_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_stage.vertexBindingDescriptionCount = 1;
	vertex_input_stage.pVertexBindingDescriptions = &vertex_binding_description;
	vertex_input_stage.vertexAttributeDescriptionCount = 4;
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
	pipe_info.layout = game->renderer_state.backend.rectangles_pipeline_layout;
	pipe_info.renderPass = game->renderer_state.backend.window_render_pass;
	pipe_info.stageCount = 2;
	pipe_info.pRasterizationState = &rasterization_state;
	pipe_info.pViewportState = &viewport_state;
	pipe_info.pDynamicState = &dynamic_state;
	pipe_info.pMultisampleState = &multi_sample_state;
	pipe_info.pInputAssemblyState = &input_assembly;

	VkResult pipeline_result = vkCreateGraphicsPipelines(game->renderer_state.backend.device, 0, 1, &pipe_info, 0, &game->renderer_state.backend.rectangles_pipeline);

	vkDestroyShaderModule(game->renderer_state.backend.device, vertex_shader, 0);
	vkDestroyShaderModule(game->renderer_state.backend.device, fragment_shader, 0);

	if (pipeline_result != VK_SUCCESS) {
		printf("[RENDERER BACKEND] Couldn't create rectangles graphics pipeline\n");
		return 1;
	}

	game->renderer_state.backend.rectangles_pipeline_usable_bool = 1;

	return 0;
}

uint32_t vulkan_device_resources_rectangles_destroy(struct minec_client* game) {

	vkDeviceWaitIdle(game->renderer_state.backend.device);

	if(game->renderer_state.backend.rectangles_pipeline_usable_bool) vkDestroyPipeline(game->renderer_state.backend.device, game->renderer_state.backend.rectangles_pipeline, 0);
	vkDestroyPipelineLayout(game->renderer_state.backend.device, game->renderer_state.backend.rectangles_pipeline_layout, 0);

	vkUnmapMemory(game->renderer_state.backend.device, game->renderer_state.backend.rectangles_buffer_memory);
	vkFreeMemory(game->renderer_state.backend.device, game->renderer_state.backend.rectangles_buffer_memory, 0);
	vkDestroyBuffer(game->renderer_state.backend.device, game->renderer_state.backend.rectangles_buffer, 0);

	return 0;
}

uint32_t vulkan_backend_add_rectangles(struct minec_client* game, struct renderer_rectangle* rectangles, uint32_t rectangles_count) {

	struct render_rectangle_vertex* vertex_buffer = (size_t)game->renderer_state.backend.rectangles_buffer_memory_host_handle + (size_t)game->renderer_state.backend.rectangles_count * sizeof(struct render_rectangle_vertex) * 6;

	for (uint32_t i = 0; i < rectangles_count; i++) {

		vertex_buffer[i * 6 + 0] = (struct render_rectangle_vertex){
			rectangles[i].x[0], rectangles[i].y[0], rectangles[i].u[0], rectangles[i].v[0], rectangles[i].image_index, rectangles[i].sampler_index
		};
		vertex_buffer[i * 6 + 1] = (struct render_rectangle_vertex){
			rectangles[i].x[1], rectangles[i].y[1], rectangles[i].u[1], rectangles[i].v[1], rectangles[i].image_index, rectangles[i].sampler_index
		};
		vertex_buffer[i * 6 + 2] = (struct render_rectangle_vertex){
			rectangles[i].x[2], rectangles[i].y[2], rectangles[i].u[2], rectangles[i].v[2], rectangles[i].image_index, rectangles[i].sampler_index
		};

		vertex_buffer[i * 6 + 3] = (struct render_rectangle_vertex){
			rectangles[i].x[2], rectangles[i].y[2], rectangles[i].u[2], rectangles[i].v[2], rectangles[i].image_index, rectangles[i].sampler_index
		};
		vertex_buffer[i * 6 + 4] = (struct render_rectangle_vertex){
			rectangles[i].x[3], rectangles[i].y[3], rectangles[i].u[3], rectangles[i].v[3], rectangles[i].image_index, rectangles[i].sampler_index
		};
		vertex_buffer[i * 6 + 5] = (struct render_rectangle_vertex){
			rectangles[i].x[0], rectangles[i].y[0], rectangles[i].u[0], rectangles[i].v[0], rectangles[i].image_index, rectangles[i].sampler_index
		};

	}

	game->renderer_state.backend.rectangles_count += rectangles_count;

	return 0;
}