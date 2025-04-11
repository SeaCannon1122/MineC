#include "pixelchar_internal.h"

#include "pixelchar_vulkan_vertex_shader.h"
#include "pixelchar_vulkan_fragment_shader.h"

struct vulkan_push_constants
{
	struct
	{
		int32_t width;
		int32_t height;
	} screen_size;

	uint32_t _padding_0[2];

	struct
	{
		float r;
		float g;
		float b;
		float a;
	} shadow_color_devisor;

	uint32_t draw_mode;

	uint32_t _padding_1[3];
};

#define PIXELCHAR_VULKAN_STAGING_SIZE 262144

#define VK_CALL_FUNCTION(call, debug, instead) do {if (call != VK_SUCCESS) {debug; instead;}} while(0)

uint32_t _pixelchar_renderer_font_init(struct pixelchar_renderer* pcr, uint32_t index)
{
	vkFreeMemory(pcr->backend.vulkan.device, pcr->backend.vulkan.font_info[index].bitmap_buffer_memory, 0);
	vkDestroyBuffer(pcr->backend.vulkan.device, pcr->backend.vulkan.font_info[index].bitmap_buffer, 0);
}

uint32_t _pixelchar_renderer_font_deinit(struct pixelchar_renderer* pcr, uint32_t index, bool from_initial_init)
{
	VkBufferCreateInfo buffer_info = { 0 };
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	buffer_info.size = pcr->font_info[index].mappings_count * pcr->font_info[index].resolution * pcr->font_info[index].resolution / 8;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(pcr->backend.vulkan.device, &buffer_info, 0, &pcr->backend.vulkan.font_info[index].bitmap_buffer) != VK_SUCCESS)
	{
		if (from_initial_init)
			_DEBUG_CALLBACK_CRITICAL_ERROR("pixelchar_renderer_backend_vulkan_init: font bitmap vkCreateBuffer failed");
		else 
			_DEBUG_CALLBACK_CRITICAL_ERROR("pixelchar_renderer_add_font: font bitmap vkCreateBuffer failed");
		return PIXELCHAR_FAILED;
	}

	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements(pcr->backend.vulkan.device, pcr->backend.vulkan.font_info[index].bitmap_buffer, &memory_requirements);
	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(pcr->backend.vulkan.gpu, &memory_properties);

	VkMemoryAllocateInfo alloc_info = { 0 };
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = memory_requirements.size;

	uint32_t found_memory_type = 0;
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
	{
		if ((memory_requirements.memoryTypeBits & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		{
			alloc_info.memoryTypeIndex = i;
			found_memory_type = 1;
			break;
		}
	}

	if (found_memory_type == 0)
	{
		if (from_initial_init)
			_DEBUG_CALLBACK_CRITICAL_ERROR("pixelchar_renderer_backend_vulkan_init: did not find suitable VkMemoryType for font bitmap buffer");
		else
			_DEBUG_CALLBACK_CRITICAL_ERROR("pixelchar_renderer_add_font: did not find suitable VkMemoryType for font bitmap buffer");
		goto vkAllocateMemory_failed;
	}

	VK_CALL_FUNCTION(
		vkAllocateMemory(pcr->backend.vulkan.device, &alloc_info, 0, &pcr->backend.vulkan.font_info[index].bitmap_buffer_memory),
		if (from_initial_init) { _DEBUG_CALLBACK_CRITICAL_ERROR("pixelchar_renderer_backend_vulkan_init: font bitmap vkAllocateMemory failed"); }
		else { _DEBUG_CALLBACK_CRITICAL_ERROR("pixelchar_renderer_add_font: font bitmap vkAllocateMemory failed"); },
		goto vkAllocateMemory_failed
	);

	VK_CALL_FUNCTION(
		vkBindBufferMemory(pcr->backend.vulkan.device, pcr->backend.vulkan.font_info[index].bitmap_buffer, pcr->backend.vulkan.font_info[index].bitmap_buffer_memory, 0),
		if (from_initial_init) { _DEBUG_CALLBACK_CRITICAL_ERROR("pixelchar_renderer_backend_vulkan_init: font bitmap vkBindBufferMemory failed"); }
		else { _DEBUG_CALLBACK_CRITICAL_ERROR("pixelchar_renderer_add_font: font bitmap vkBindBufferMemory failed"); },
		goto vkBindBufferMemory_failed
	);

	VkDeviceSize offset = 0;
	VkDeviceSize total_size = pcr->font_info[index].widths_count * pcr->font_info[index].resolution * pcr->font_info[index].resolution / 8;

	while (offset < total_size)
	{
		VkDeviceSize chunk_size = (PIXELCHAR_VULKAN_STAGING_SIZE < total_size - offset ? PIXELCHAR_VULKAN_STAGING_SIZE : total_size - offset);

		offset += chunk_size;
	}

	return PIXELCHAR_SUCCESS;

vkBindBufferMemory_failed:
	vkFreeMemory(pcr->backend.vulkan.device, pcr->backend.vulkan.font_info[index].bitmap_buffer_memory, 0);
vkAllocateMemory_failed:
	vkDestroyBuffer(pcr->backend.vulkan.device, pcr->backend.vulkan.font_info[index].bitmap_buffer, 0);
vkCreateBuffer_filed:
	return PIXELCHAR_FAILED;
}

uint32_t pixelchar_renderer_backend_vulkan_init(
	struct pixelchar_renderer* pcr,
	VkDevice device,
	VkPhysicalDevice gpu,
	VkQueue queue,
	uint32_t queue_index,
	VkRenderPass render_pass,
	uint8_t* vertex_shader_custom,
	uint32_t vertex_shader_custom_length,
	uint8_t* fragment_shader_custom,
	uint32_t fragment_shader_custom_length
)
{
	if (pcr->backend_selected != PIXELCHAR_RENDERER_BACKEND_NONE)
	{
		_DEBUG_CALLBACK_WARNING("pixelchar_renderer_backend_vulkan_init: backend should be deinitialized by the user before initializing another backend");
	}

	vkDeviceWaitIdle(device);

	struct pixelchar_vulkan_backend backend;
	backend.cmd_set = 0;
	backend.device = device;
	backend.gpu = gpu;
	backend.queue = queue;
	backend.queue_index = queue_index;

	VkShaderModule vertex_shader, fragment_shader;

	VkShaderModuleCreateInfo shader_info = { 0 };
	shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

	shader_info.pCode = (vertex_shader_custom == 0 ? vertex_shader_default : vertex_shader_custom);
	shader_info.codeSize = (vertex_shader_custom == 0 ? vertex_shader_default_len : vertex_shader_custom_length);

	VK_CALL_FUNCTION(
		vkCreateShaderModule(backend.device, &shader_info, 0, &vertex_shader),
		_DEBUG_CALLBACK_CRITICAL_ERROR("pixelchar_renderer_backend_vulkan_init: vertex vkCreateShaderModule failed"),
		goto vertex_vkCreateShaderModule_failed
	);

	shader_info.pCode = (fragment_shader_custom == 0 ? fragment_shader_default : fragment_shader_custom);
	shader_info.codeSize = (fragment_shader_custom == 0 ? fragment_shader_default_len : fragment_shader_custom_length);

	VK_CALL_FUNCTION(
		vkCreateShaderModule(backend.device, &shader_info, 0, &fragment_shader),
		_DEBUG_CALLBACK_CRITICAL_ERROR("pixelchar_renderer_backend_vulkan_init: fragment vkCreateShaderModule failed"),
		goto fragment_vkCreateShaderModule_failed
	);

	VkDescriptorSetLayoutBinding pixelfont_buffer_binding = { 0 };
	pixelfont_buffer_binding.descriptorCount = 1;
	pixelfont_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	pixelfont_buffer_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding bindings[PIXELCHAR_RENDERER_MAX_FONTS];
	for (int32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONTS; i++) 
	{
		pixelfont_buffer_binding.binding = i;
		bindings[i] = pixelfont_buffer_binding;
	}

	VkDescriptorSetLayoutCreateInfo set_layout_info = { 0 };
	set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set_layout_info.bindingCount = PIXELCHAR_RENDERER_MAX_FONTS;
	set_layout_info.pBindings = bindings;

	VK_CALL_FUNCTION(
		vkCreateDescriptorSetLayout(backend.device, &set_layout_info, 0, &backend.set_layout),
		_DEBUG_CALLBACK_CRITICAL_ERROR("pixelchar_renderer_backend_vulkan_init: vkCreateDescriptorSetLayout failed"),
		goto vkCreateDescriptorSetLayout_failed
	);

	VkPushConstantRange push_constant_range = { 0 };
	push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	push_constant_range.offset = 0;
	push_constant_range.size = sizeof(struct vulkan_push_constants);

	VkPipelineLayoutCreateInfo pipeline_layout_info = { 0 };
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &backend.set_layout;
	pipeline_layout_info.pushConstantRangeCount = 1;
	pipeline_layout_info.pPushConstantRanges = &push_constant_range;

	VK_CALL_FUNCTION(
		vkCreatePipelineLayout(backend.device, &pipeline_layout_info, 0, &backend.pipe_layout),
		_DEBUG_CALLBACK_CRITICAL_ERROR("pixelchar_renderer_backend_vulkan_init: vkCreatePipelineLayout failed"),
		goto vkCreatePipelineLayout_failed
	);

	VkVertexInputBindingDescription vertex_binding_description = { 0 };
	vertex_binding_description.binding = 0;
	vertex_binding_description.stride = sizeof(struct internal_pixelchar);
	vertex_binding_description.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;


	VkVertexInputAttributeDescription vertex_attribute_descriptions[] = 
	{

		{
			.binding = 0,
			.location = 0,
			.format = VK_FORMAT_R8G8B8A8_UNORM,
			.offset = offsetof(struct internal_pixelchar, color)
		},

		{
			.binding = 0,
			.location = 1,
			.format = VK_FORMAT_R8G8B8A8_UNORM,
			.offset = offsetof(struct internal_pixelchar, background_color)
		},

		{
			.binding = 0,
			.location = 2,
			.format = VK_FORMAT_R32_UINT,
			.offset = offsetof(struct internal_pixelchar, bitmap_index)
		},

		{
			.binding = 0,
			.location = 3,
			.format = VK_FORMAT_R8_UINT,
			.offset = offsetof(struct internal_pixelchar, font)
		},

		{
			.binding = 0,
			.location = 4,
			.format = VK_FORMAT_R8_UINT,
			.offset = offsetof(struct internal_pixelchar, bitmap_width)
		},

		{
			.binding = 0,
			.location = 5,
			.format = VK_FORMAT_R8_UINT,
			.offset = offsetof(struct internal_pixelchar, masks)
		},

		{
			.binding = 0,
			.location = 6,
			.format = VK_FORMAT_R8_UINT,
			.offset = offsetof(struct internal_pixelchar, font_resolution)
		},

		{
			.binding = 0,
			.location = 7,
			.format = VK_FORMAT_R16G16B16A16_SINT,
			.offset = offsetof(struct internal_pixelchar, position)
		},

	};


	VkPipelineVertexInputStateCreateInfo vertex_input_stage = { 0 };
	vertex_input_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_stage.vertexBindingDescriptionCount = 1;
	vertex_input_stage.pVertexBindingDescriptions = &vertex_binding_description;
	vertex_input_stage.vertexAttributeDescriptionCount = 8;
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
	pipe_info.layout = backend.pipe_layout;
	pipe_info.renderPass = render_pass;
	pipe_info.stageCount = 2;
	pipe_info.pRasterizationState = &rasterization_state;
	pipe_info.pViewportState = &viewport_state;
	pipe_info.pDynamicState = &dynamic_state;
	pipe_info.pMultisampleState = &multi_sample_state;
	pipe_info.pInputAssemblyState = &input_assembly;

	VK_CALL_FUNCTION(
		vkCreateGraphicsPipelines(backend.device, 0, 1, &pipe_info, 0, &backend.pipeline),
		_DEBUG_CALLBACK_CRITICAL_ERROR("pixelchar_renderer_backend_vulkan_init: vkCreateGraphicsPipelines failed"),
		goto vkCreateGraphicsPipelines_failed
	);
	
	VkCommandPoolCreateInfo pool_info = { 0 };
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = backend.queue_index;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VK_CALL_FUNCTION(
		vkCreateCommandPool(backend.device, &pool_info, 0, &backend.transfer_command_pool),
		_DEBUG_CALLBACK_CRITICAL_ERROR("pixelchar_renderer_backend_vulkan_init: vkCreateCommandPool failed"),
		goto vkCreateCommandPool_failed
	);

	VkCommandBufferAllocateInfo cmd_alloc_info = { 0 };
	cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd_alloc_info.commandBufferCount = 1;
	cmd_alloc_info.commandPool = backend.transfer_command_pool;
	cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VK_CALL_FUNCTION(
		vkAllocateCommandBuffers(backend.device, &cmd_alloc_info, &backend.transfer_cmd),
		_DEBUG_CALLBACK_CRITICAL_ERROR("pixelchar_renderer_backend_vulkan_init: vkAllocateCommandBuffers failed"),
		goto vkAllocateCommandBuffers_failed
	);

	size_t vertex_index_buffer_size = pcr->char_buffer_length * (sizeof(struct internal_pixelchar) + sizeof(uint16_t) * 6);
	size_t staging_buffer_size = PIXELCHAR_VULKAN_STAGING_SIZE;
	size_t vertex_index_staging_memory_size = vertex_index_buffer_size + staging_buffer_size;

	VkBufferCreateInfo buffer_info = { 0 };
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	buffer_info.size = vertex_index_buffer_size;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CALL_FUNCTION(
		vkCreateBuffer(backend.device, &buffer_info, 0, &backend.vertex_index_buffer),
		_DEBUG_CALLBACK_CRITICAL_ERROR("pixelchar_renderer_backend_vulkan_init: vertex index vkCreateBuffer failed"),
		goto vertex_index_vkCreateBuffer_failed
	);

	buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	buffer_info.size = staging_buffer_size;

	VK_CALL_FUNCTION(
		vkCreateBuffer(backend.device, &buffer_info, 0, &backend.staging_buffer),
		_DEBUG_CALLBACK_CRITICAL_ERROR("pixelchar_renderer_backend_vulkan_init: staging vkCreateBuffer failed"),
		goto vertex_index_vkCreateBuffer_failed
	);

	VkMemoryRequirements vertex_index_memory_requirements;
	vkGetBufferMemoryRequirements(backend.device, backend.vertex_index_buffer, &vertex_index_memory_requirements);
	VkMemoryRequirements staging_memory_requirements;
	vkGetBufferMemoryRequirements(backend.device, backend.staging_buffer, &staging_memory_requirements);

	VkDeviceSize offset = (vertex_index_memory_requirements.size + staging_memory_requirements.alignment - 1) & ~(staging_memory_requirements.alignment - 1);
	VkDeviceSize memory_size = offset + staging_memory_requirements.size;

	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(backend.gpu, &memory_properties);

	VkMemoryAllocateInfo alloc_info = { 0 };
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = memory_size;

	uint32_t found_memory_type = 0;
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) 
	{
		if (((vertex_index_memory_requirements.memoryTypeBits & staging_memory_requirements.memoryTypeBits) & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) == (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
		{
			alloc_info.memoryTypeIndex = i;
			found_memory_type = 1;
			break;
		}
	}

	if (found_memory_type == 0)
	{
		_DEBUG_CALLBACK_CRITICAL_ERROR("pixelchar_renderer_backend_vulkan_init: did not find suitable VkMemoryType for vertex staging and index buffer");
		goto vkAllocateMemory_failed;
	}

	VK_CALL_FUNCTION(
		vkAllocateMemory(backend.device, &alloc_info, 0, &backend.vertex_index_staging_buffer_memory),
		_DEBUG_CALLBACK_CRITICAL_ERROR("pixelchar_renderer_backend_vulkan_init: vkAllocateMemory failed"),
		goto vkAllocateMemory_failed
	);

	VK_CALL_FUNCTION(
		vkBindBufferMemory(backend.device, backend.vertex_index_buffer, backend.vertex_index_staging_buffer_memory, 0),
		_DEBUG_CALLBACK_CRITICAL_ERROR("pixelchar_renderer_backend_vulkan_init: vertex index vkBindBufferMemory failed"),
		goto vertex_index_vkBindBufferMemory_failed
	);

	VK_CALL_FUNCTION(
		vkBindBufferMemory(backend.device, backend.staging_buffer, backend.vertex_index_staging_buffer_memory, offset),
		_DEBUG_CALLBACK_CRITICAL_ERROR("pixelchar_renderer_backend_vulkan_init: staging vkBindBufferMemory failed"),
		goto staging_vkBindBufferMemory_failed
	);

	VK_CALL_FUNCTION(
		vkMapMemory(backend.device, backend.vertex_index_staging_buffer_memory, 0, memory_size, 0, &backend.vertex_index_staging_buffer_host_handle),
		_DEBUG_CALLBACK_CRITICAL_ERROR("pixelchar_renderer_backend_vulkan_init: vkMapMemory failed"),
		goto vkMapMemory_failed
	);

	VkDescriptorPoolSize pool_size = { 0 };
	pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	pool_size.descriptorCount = PIXELCHAR_RENDERER_MAX_FONTS;

	VkDescriptorPoolCreateInfo descriptor_pool_info = { 0 };
	descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_pool_info.maxSets = 1;
	descriptor_pool_info.poolSizeCount = 1;
	descriptor_pool_info.pPoolSizes = &pool_size;

	VK_CALL_FUNCTION(
		vkCreateDescriptorPool(backend.device, &descriptor_pool_info, 0, &backend.descriptor_pool),
		_DEBUG_CALLBACK_CRITICAL_ERROR("pixelchar_renderer_backend_vulkan_init: vkCreateDescriptorPool failed"),
		goto vkCreateDescriptorPool_failed
	);

	VkDescriptorSetAllocateInfo descriptor_set_info = { 0 };
	descriptor_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptor_set_info.pSetLayouts = &backend.set_layout;
	descriptor_set_info.descriptorSetCount = 1;
	descriptor_set_info.descriptorPool = backend.descriptor_pool;

	VK_CALL_FUNCTION(
		vkAllocateDescriptorSets(backend.device, &descriptor_set_info, &backend.descriptor_set),
		_DEBUG_CALLBACK_CRITICAL_ERROR("pixelchar_renderer_backend_vulkan_init: vkAllocateDescriptorSets failed"),
		goto vkAllocateDescriptorSets_failed
	);

	uint16_t* index_buffer_ptr = (size_t)backend.vertex_index_staging_buffer_host_handle + sizeof(struct internal_pixelchar) * pcr->char_buffer_length;
	for (uint32_t i = 0; i < pcr->char_buffer_length; i++)
	{
		index_buffer_ptr[i * 6 + 0] = 4 * i;
		index_buffer_ptr[i * 6 + 1] = 4 * i + 1;
		index_buffer_ptr[i * 6 + 2] = 4 * i + 2;
		index_buffer_ptr[i * 6 + 3] = 4 * i + 1;
		index_buffer_ptr[i * 6 + 4] = 4 * i + 3;
		index_buffer_ptr[i * 6 + 5] = 4 * i + 2;
	}

	vkDestroyShaderModule(backend.device, vertex_shader, 0);
	vkDestroyShaderModule(backend.device, fragment_shader, 0);

	if (pcr->backend_selected != PIXELCHAR_RENDERER_BACKEND_NONE)
	{
		pixelchar_renderer_backend_deinit(pcr);
	}

	pcr->backend.vulkan = backend;
	pcr->backend_selected = PIXELCHAR_RENDERER_BACKEND_VULKAN;

	return PIXELCHAR_SUCCESS;

vkAllocateDescriptorSets_failed:
	vkDestroyDescriptorPool(backend.device, backend.descriptor_pool, 0);
vkCreateDescriptorPool_failed:
	vkUnmapMemory(backend.device, backend.vertex_index_staging_buffer_memory);
vkMapMemory_failed:
staging_vkBindBufferMemory_failed:
vertex_index_vkBindBufferMemory_failed:
	vkFreeMemory(backend.device, backend.vertex_index_staging_buffer_memory, 0);
vkAllocateMemory_failed:
	vkDestroyBuffer(backend.device, backend.staging_buffer, 0);
staging_vkAllocateMemory_failed:
	vkDestroyBuffer(backend.device, backend.vertex_index_buffer, 0);
vertex_index_vkCreateBuffer_failed:
	vkFreeCommandBuffers(backend.device, backend.transfer_command_pool, 1, &backend.transfer_cmd);
vkAllocateCommandBuffers_failed:
	vkDestroyCommandPool(backend.device, backend.transfer_command_pool, 0);
vkCreateCommandPool_failed:
	vkDestroyPipeline(backend.device, backend.pipeline, 0);
vkCreateGraphicsPipelines_failed:
	vkDestroyPipelineLayout(backend.device, backend.pipe_layout, 0);
vkCreatePipelineLayout_failed:
	vkDestroyDescriptorSetLayout(backend.device, backend.set_layout, 0);
vkCreateDescriptorSetLayout_failed:
	vkDestroyShaderModule(backend.device, fragment_shader, 0);
fragment_vkCreateShaderModule_failed:
	vkDestroyShaderModule(backend.device, vertex_shader, 0);
vertex_vkCreateShaderModule_failed:

	return PIXELCHAR_FAILED;
}

void _pixelchar_renderer_backend_vulkan_deinit(struct pixelchar_renderer* pcr)
{
	for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONTS; i++) if (pcr->backend.vulkan.font_info[i].init)
	{
		
	}

	vkDeviceWaitIdle(pcr->backend.vulkan.device);

	vkDestroyDescriptorPool(pcr->backend.vulkan.device, pcr->backend.vulkan.descriptor_pool, 0);

	vkUnmapMemory(pcr->backend.vulkan.device, pcr->backend.vulkan.vertex_index_staging_buffer_memory);
	vkFreeMemory(pcr->backend.vulkan.device, pcr->backend.vulkan.vertex_index_staging_buffer_memory, 0);
	vkDestroyBuffer(pcr->backend.vulkan.device, pcr->backend.vulkan.vertex_index_buffer, 0);
	vkDestroyBuffer(pcr->backend.vulkan.device, pcr->backend.vulkan.staging_buffer, 0);

	vkFreeCommandBuffers(pcr->backend.vulkan.device, pcr->backend.vulkan.transfer_command_pool, 1, &pcr->backend.vulkan.transfer_cmd);
	vkDestroyCommandPool(pcr->backend.vulkan.device, pcr->backend.vulkan.transfer_command_pool, 0);

	vkDestroyPipeline(pcr->backend.vulkan.device, pcr->backend.vulkan.pipeline, 0);
	vkDestroyPipelineLayout(pcr->backend.vulkan.device, pcr->backend.vulkan.pipe_layout, 0);
	vkDestroyDescriptorSetLayout(pcr->backend.vulkan.device, pcr->backend.vulkan.set_layout, 0);

	return PIXELCHAR_SUCCESS;
}

uint32_t _pixelchar_renderer_backend_vulkan_add_font(struct pixelchar_renderer* pcr, void* font, uint32_t index)
{

}

void pixelchar_renderer_backend_vulkan_render(struct pixelchar_renderer* pcr, VkCommandBuffer cmd, uint32_t width, uint32_t height)
{
	if (pcr->backend_selected != _PIXELCHAR_BACKEND_VULKAN) _DEBUG_CALLBACK_ERROR_RETURN("pixelchar_renderer_backend_vulkan_render: vulkan backend not initialized");

	memcpy(pcr->backend.vulkan.vertex_index_staging_buffer_host_handle, pcr->char_buffer, sizeof(struct internal_pixelchar) * pcr->char_count);

	vkCmdBindDescriptorSets(
		cmd,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pcr->backend.vulkan.pipe_layout,
		0,
		1,
		&pcr->backend.vulkan.descriptor_set,
		0,
		0
	);

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pcr->backend.vulkan.pipeline);

	VkDeviceSize device_size = 0;
	vkCmdBindVertexBuffers(cmd, 0, 1, &pcr->backend.vulkan.vertex_index_buffer, &device_size);
	vkCmdBindIndexBuffer(cmd, pcr->backend.vulkan.vertex_index_buffer, sizeof(struct internal_pixelchar) * pcr->char_buffer_length, VK_INDEX_TYPE_UINT16);

	struct vulkan_push_constants push_constants;
	push_constants.screen_size.width = width;
	push_constants.screen_size.height = height;
	push_constants.shadow_color_devisor.r = 4.0;
	push_constants.shadow_color_devisor.g = 4.0;
	push_constants.shadow_color_devisor.b = 4.0;
	push_constants.shadow_color_devisor.a = 1.4;

	for (uint32_t char_render_mode = 0; char_render_mode < 3; char_render_mode++)
	{
		push_constants.draw_mode = char_render_mode;

		vkCmdPushConstants(
			cmd,
			pcr->backend.vulkan.pipe_layout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(struct vulkan_push_constants),
			&push_constants
		);

		vkCmdDrawIndexed(cmd, 6, pcr->char_count, 0, 0, 0);
	}
}