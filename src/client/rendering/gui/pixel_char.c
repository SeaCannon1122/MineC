#include "pixel_char.h"

#include <stdio.h>
#include <malloc.h>

#define __PIXEL_CHAR_WIDTH(c, font_map) (((struct pixel_font*)((font_map)[(c).masks & PIXEL_CHAR_FONT_MASK]))->char_font_entries[(c).value].width)

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

uint32_t pixel_char_renderer_new(struct pixel_char_renderer* pcr, struct rendering_memory_manager* rmm, VkDevice device, VkRenderPass render_pass) {

	pcr->device = device;
	pcr->font_count = 0;

	VkDescriptorSetLayoutBinding pixel_char_buffer_binding = { 0 };
	pixel_char_buffer_binding.binding = 0;
	pixel_char_buffer_binding.descriptorCount = 1;
	pixel_char_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	pixel_char_buffer_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding pixel_font_buffer_binding = { 0 };
	pixel_font_buffer_binding.descriptorCount = 1;
	pixel_font_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	pixel_font_buffer_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding bindings[MAX_PIXEL_FONTS + 1] = {
		pixel_char_buffer_binding,
	};

	for (int32_t i = 0; i < MAX_PIXEL_FONTS; i++) {
		pixel_font_buffer_binding.binding = i + 1;
		bindings[i + 1] = pixel_font_buffer_binding;
	}

	VkDescriptorSetLayoutCreateInfo set_layout_info = { 0 };
	set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set_layout_info.bindingCount = MAX_PIXEL_FONTS + 1;
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

	VKCall(new_VkShaderModule(pcr->device, "../../../resources/vertex_shader.spv", &vertex_shader));
	VKCall(new_VkShaderModule(pcr->device, "../../../resources/fragment_shader.spv", &fragment_shader));

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
	pipe_info.layout = pcr->pipe_layout;
	pipe_info.renderPass = render_pass;
	pipe_info.stageCount = 2;
	pipe_info.pRasterizationState = &rasterization_state;
	pipe_info.pViewportState = &viewport_state;
	pipe_info.pDynamicState = &dynamic_state;
	pipe_info.pMultisampleState = &multi_sample_state;
	pipe_info.pInputAssemblyState = &input_assembly;

	VKCall(vkCreateGraphicsPipelines(pcr->device, 0, 1, &pipe_info, 0, &pcr->pipeline));

	vkDestroyShaderModule(pcr->device, vertex_shader, 0);
	vkDestroyShaderModule(pcr->device, fragment_shader, 0);


	VKCall(VkBuffer_new(rmm, sizeof(struct pixel_render_char) * 16384, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, &pcr->pixel_char_buffer));


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

	VkDescriptorBufferInfo pixel_char_buffer_info = { 0 };
	pixel_char_buffer_info.buffer = pcr->pixel_char_buffer.buffer;
	pixel_char_buffer_info.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet pixel_char_buffer_write = { 0 };
	pixel_char_buffer_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	pixel_char_buffer_write.dstSet = pcr->descriptor_set;
	pixel_char_buffer_write.pBufferInfo = &pixel_char_buffer_info;
	pixel_char_buffer_write.dstBinding = 0;
	pixel_char_buffer_write.descriptorCount = 1;
	pixel_char_buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

	vkUpdateDescriptorSets(pcr->device, 1, &pixel_char_buffer_write, 0, 0);


	return 0;
}

uint32_t pixel_char_renderer_add_font(struct pixel_char_renderer* pcr, struct rendering_memory_manager* rmm, struct pixel_font* font_data) {

	if (pcr->font_count >= MAX_PIXEL_FONTS) return 1;

	VKCall(VkBuffer_new(rmm, sizeof(struct pixel_font), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, &pcr->pixel_font_buffer[pcr->font_count]));
	VKCall(VkBuffer_fill(rmm, &pcr->pixel_font_buffer[pcr->font_count], font_data, sizeof(struct pixel_font)));

	VkDescriptorBufferInfo pixel_font_buffer_info = { 0 };
	pixel_font_buffer_info.buffer = pcr->pixel_font_buffer[pcr->font_count].buffer;
	pixel_font_buffer_info.range = VK_WHOLE_SIZE;

	pcr->font_count++;

	VkWriteDescriptorSet pixel_font_buffer_write = { 0 };
	pixel_font_buffer_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	pixel_font_buffer_write.dstSet = pcr->descriptor_set;
	pixel_font_buffer_write.pBufferInfo = &pixel_font_buffer_info;
	pixel_font_buffer_write.dstBinding = pcr->font_count;
	pixel_font_buffer_write.descriptorCount = 1;
	pixel_font_buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

	vkUpdateDescriptorSets(pcr->device, 1, &pixel_font_buffer_write, 0, 0);

	return 0;
}

uint32_t pixel_char_renderer_fill_chars(struct pixel_char_renderer* pcr, struct rendering_memory_manager* rmm, struct pixel_render_char* chars, uint32_t chars_count) {
	pcr->chars_to_draw = chars_count;

	VKCall(VkBuffer_fill(rmm, &pcr->pixel_char_buffer, chars, sizeof(struct pixel_render_char) * chars_count));

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

		vkCmdDraw(cmd, pcr->chars_to_draw * 6, 1, 0, 0);
	}

}