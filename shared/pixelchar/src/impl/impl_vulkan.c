#include <pixelchar/impl/impl_vulkan.h>

#include "vulkan_vertex_shader.h"
#include "vulkan_fragment_shader.h"

#include <stdlib.h>
#include <string.h>

#define STAGING_BUFFER_SIZE UINT16_MAX

struct memory_property_property_flags
{
	VkMemoryPropertyFlags include[8];
	VkMemoryPropertyFlags exclude[8];
	uint32_t count;
};

struct _push_constants
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

VkResult _allocate_best_memory(
	PixelcharImplVulkanFactory* factory,
	struct memory_property_property_flags* property_flags,
	PixelcharImplVulkanInternalBufferAndMemory* buffer_and_memory
)
{
	VkMemoryAllocateInfo allocInfo = { 0 };
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = NULL;
	allocInfo.allocationSize = buffer_and_memory->memoryRequirements.size;

	for (uint32_t pass = 0; pass < property_flags->count; pass++)
	{
		for (uint32_t i = 0; i < factory->memoryProperties.memoryTypeCount; i++ ) if (
			buffer_and_memory->memoryRequirements.memoryTypeBits & (1 << i) &&
			(factory->memoryProperties.memoryTypes[i].propertyFlags & property_flags->include[pass]) == property_flags->include[pass] &&
			!(factory->memoryProperties.memoryTypes[i].propertyFlags & property_flags->exclude[pass])
		)
		{
			allocInfo.memoryTypeIndex = i;

			buffer_and_memory->memorySize = buffer_and_memory->memoryRequirements.size;
			buffer_and_memory->memoryPropertyFlags = factory->memoryProperties.memoryTypes[i].propertyFlags;

			VkResult result = factory->func.vkAllocateMemory(factory->device, &allocInfo, NULL, &buffer_and_memory->memory);
			if (result == VK_SUCCESS || result == VK_ERROR_DEVICE_LOST) return result;
		}
	}
	return VK_ERROR_OUT_OF_DEVICE_MEMORY;
}

VkResult _create_buffer_and_memory(
	PixelcharImplVulkanFactory* factory,
	VkDeviceSize size,
	VkBufferUsageFlags usage_flags,
	struct memory_property_property_flags* property_flags,
	PixelcharImplVulkanInternalBufferAndMemory* buffer_and_memory
)
{
	VkBufferCreateInfo buffer_info = { 0 };
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buffer_info.size = size;
	buffer_info.usage = usage_flags;

	buffer_and_memory->bufferSize = size;

	VkResult result = VK_SUCCESS;
	if ((result = factory->func.vkCreateBuffer(factory->device, &buffer_info, 0, &buffer_and_memory->buffer)) != VK_SUCCESS) return result;

	factory->func.vkGetBufferMemoryRequirements(factory->device, buffer_and_memory->buffer, &buffer_and_memory->memoryRequirements);

	if ((result = _allocate_best_memory(
		factory,
		property_flags,
		buffer_and_memory
	)) != VK_SUCCESS)
	{
		factory->func.vkDestroyBuffer(factory->device, buffer_and_memory->buffer, 0);
		return result;
	}

	if ((result = factory->func.vkBindBufferMemory(factory->device, buffer_and_memory->buffer, buffer_and_memory->memory, 0)) != VK_SUCCESS)
	{
		factory->func.vkDestroyBuffer(factory->device, buffer_and_memory->buffer, 0);
		factory->func.vkFreeMemory(factory->device, buffer_and_memory->memory, 0);
		return result;
	}

	return VK_SUCCESS;
}

void _destroy_buffer_and_memory(PixelcharImplVulkanFactory* factory, PixelcharImplVulkanInternalBufferAndMemory* buffer_and_memory)
{
	factory->func.vkDestroyBuffer(factory->device, buffer_and_memory->buffer, 0);
	factory->func.vkFreeMemory(factory->device, buffer_and_memory->memory, 0);
}

VkResult _cmd_copy_data(PixelcharImplVulkanFactory* factory, void* data, size_t data_size, VkBuffer buffer, uint32_t offset)
{
	VkResult result;
	VkDeviceSize chunk_size;

	if ((result = factory->func.vkResetCommandBuffer(factory->cmd, 0)) != VK_SUCCESS) return result;
	if ((result = factory->func.vkResetFences(factory->device, 1, &factory->fence)) != VK_SUCCESS) return result;

	VkCommandBufferBeginInfo begin_info = { 0 };
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	for (VkDeviceSize offset = 0; offset < data_size; offset += chunk_size)
	{
		if ((result = factory->func.vkBeginCommandBuffer(factory->cmd, &begin_info)) != VK_SUCCESS) return result;

		chunk_size = (factory->staging.bufferSize < data_size - offset ? factory->staging.bufferSize : data_size - offset);
		memcpy(factory->stagingBufferHostHandle, (uintptr_t)data + offset, chunk_size);

		if (factory->staging.memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT == 0)
		{
			VkMappedMemoryRange range = { 0 };
			range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			range.memory = factory->staging.memory;
			range.size = factory->staging.memorySize;

			if ((result = vkFlushMappedMemoryRanges(factory->device, 1, &range)) != VK_SUCCESS) return result;
		}

		VkBufferCopy copy = { 0 };
		copy.srcOffset = 0;
		copy.size = chunk_size;
		copy.dstOffset = offset;

		factory->func.vkCmdCopyBuffer(factory->cmd, factory->staging.buffer, buffer, 1, &copy);

		if ((result = factory->func.vkEndCommandBuffer(factory->cmd)) != VK_SUCCESS) return result;
		if ((result = factory->func.vkQueueWaitIdle(factory->queue)) != VK_SUCCESS) return result;

		VkSubmitInfo submit_info = { 0 };
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &factory->cmd;

		if ((result = factory->func.vkQueueSubmit(factory->queue, 1, &submit_info, factory->fence)) != VK_SUCCESS) return result;
		if ((result = factory->func.vkWaitForFences(factory->device, 1, &factory->fence, VK_TRUE, UINT64_MAX)) != VK_SUCCESS) return result;
	}
	
	return VK_SUCCESS;
}

VkResult PixelcharImplVulkanFactoryCreate(
	VkInstance instance,
	VkDevice device,
	VkPhysicalDevice physicalDevice,
	uint32_t queueFamilyIndex,
	VkQueue queue,
	PFN_vkGetInstanceProcAddr pfnvkGetInstanceProcAddr,
	PixelcharImplVulkanFactory* pFactory,
	PixelcharLogCallback logCallback,
	void* logCallbackUserParam
)
{
	pFactory->instance = instance;
	pFactory->device = device;
	pFactory->physicalDevice = physicalDevice;
	pFactory->queueFamilyIndex = queueFamilyIndex;
	pFactory->queue = queue;

	VkResult result = VK_SUCCESS;

	bool
		index_buffer_and_memory_created = false,
		staging_buffer_and_memory_created = false,
		descriptor_set_layout_created = false,
		pipeline_layout_created = false,
		command_pool_created = false,
		command_buffers_allocated = false,
		fence_created = false,
		memory_mapped = false
	;

	struct load_entry { void** load_dst; uint8_t* func_name; };

	struct load_entry instance_load_entries[] =
	{
#define PIXELCHAR_IMPL_VULKAN_INTERNAL_INSTANCE_FUNCTION(name) {(void**)&pFactory->func.name, #name},
			PIXELCHAR_IMPL_VULKAN_INTERNAL_INSTANCE_FUNCTION_LIST
#undef PIXELCHAR_IMPL_VULKAN_INTERNAL_INSTANCE_FUNCTION
	};

	struct load_entry device_load_entries[] =
	{
#define PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(name) {(void**)&pFactory->func.name, #name},
			PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION_LIST
#undef PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION
	};

	for (uint32_t i = 0; i < sizeof(instance_load_entries) / sizeof(instance_load_entries[0]) && result == VK_SUCCESS; i++)
		if ((*instance_load_entries[i].load_dst = (void*)pfnvkGetInstanceProcAddr(pFactory->instance, instance_load_entries[i].func_name)) == NULL)
			result = VK_ERROR_UNKNOWN;

	for (uint32_t i = 0; i < sizeof(device_load_entries) / sizeof(device_load_entries[0]) && result == VK_SUCCESS; i++)
		if ((*device_load_entries[i].load_dst = (void*)pFactory->func.vkGetDeviceProcAddr(pFactory->device, device_load_entries[i].func_name)) == NULL)
			result = VK_ERROR_UNKNOWN;

	if (result == VK_SUCCESS)
	{
		pFactory->func.vkGetPhysicalDeviceMemoryProperties(pFactory->physicalDevice, &pFactory->memoryProperties);
		pFactory->func.vkGetPhysicalDeviceProperties(pFactory->physicalDevice, &pFactory->deviceProperties);
	}
	if (result == VK_SUCCESS)
	{
		if ((result = _create_buffer_and_memory(
			pFactory,
			sizeof(uint16_t) * 6,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			&(struct memory_property_property_flags) {
			.count = 3,
				.include = { VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0 },
				.exclude = { VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 0, 0 }
			},
			&pFactory->index
		)) == VK_SUCCESS) index_buffer_and_memory_created = true;
	}
	if (result == VK_SUCCESS)
	{
		if ((result = _create_buffer_and_memory(
			pFactory,
			STAGING_BUFFER_SIZE,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			&(struct memory_property_property_flags) {
			.count = 2,
				.include = { VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT },
				.exclude = { 0, 0 }
			},
			&pFactory->staging
		)) == VK_SUCCESS) staging_buffer_and_memory_created = true;
	}
	if (result == VK_SUCCESS)
	{
		VkDescriptorSetLayoutBinding pixelfont_buffer_binding = { 0 };
		pixelfont_buffer_binding.descriptorCount = 1;
		pixelfont_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		pixelfont_buffer_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		pixelfont_buffer_binding.binding = 0;

		VkDescriptorSetLayoutCreateInfo set_layout_info = { 0 };
		set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		set_layout_info.bindingCount = 1;
		set_layout_info.pBindings = &pixelfont_buffer_binding;

		if ((result = pFactory->func.vkCreateDescriptorSetLayout(pFactory->device, &set_layout_info, 0, &pFactory->descriptorSetLayout)) == VK_SUCCESS) descriptor_set_layout_created = true;
	}
	if (result == VK_SUCCESS)
	{
		VkPushConstantRange push_constant_range = { 0 };
		push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		push_constant_range.offset = 0;
		push_constant_range.size = sizeof(struct _push_constants);

		VkPipelineLayoutCreateInfo pipeline_layout_info = { 0 };
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount = 1;
		pipeline_layout_info.pSetLayouts = &pFactory->descriptorSetLayout;
		pipeline_layout_info.pushConstantRangeCount = 1;
		pipeline_layout_info.pPushConstantRanges = &push_constant_range;

		if ((result = pFactory->func.vkCreatePipelineLayout(pFactory->device, &pipeline_layout_info, 0, &pFactory->pipelineLayout)) == VK_SUCCESS) pipeline_layout_created = true;
	}
	if (result == VK_SUCCESS)
	{
		VkCommandPoolCreateInfo pool_info = { 0 };
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.queueFamilyIndex = pFactory->queueFamilyIndex;
		pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if ((result = pFactory->func.vkCreateCommandPool(pFactory->device, &pool_info, 0, &pFactory->cmdPool)) == VK_SUCCESS) command_pool_created = true;
	}
	if (result == VK_SUCCESS)
	{
		VkCommandBufferAllocateInfo cmd_alloc_info = { 0 };
		cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd_alloc_info.commandBufferCount = 1;
		cmd_alloc_info.commandPool = pFactory->cmdPool;
		cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		if ((result = pFactory->func.vkAllocateCommandBuffers(pFactory->device, &cmd_alloc_info, &pFactory->cmd)) == VK_SUCCESS) command_buffers_allocated = true;
	}
	if (result == VK_SUCCESS)
	{
		VkFenceCreateInfo fence_info = { 0 };
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if ((result = pFactory->func.vkCreateFence(pFactory->device, &fence_info, 0, &pFactory->fence)) == VK_SUCCESS) fence_created = true;
	}
	if (result == VK_SUCCESS)
	{
		if ((result = pFactory->func.vkMapMemory(
			pFactory->device,
			pFactory->staging.memory,
			0,
			VK_WHOLE_SIZE,
			0,
			&pFactory->stagingBufferHostHandle
		)) == VK_SUCCESS) memory_mapped = true;
	}
	if (result == VK_SUCCESS)
	{
		uint16_t index_data[] = { 0, 1, 2, 1, 3, 2 };
		result = _cmd_copy_data(pFactory, index_data, sizeof(index_data), pFactory->index.buffer, 0);
	}

	if (result != VK_SUCCESS)
	{
		if (memory_mapped) pFactory->func.vkUnmapMemory(pFactory->device, pFactory->staging.memory);
		if (fence_created) pFactory->func.vkDestroyFence(pFactory->device, pFactory->fence, 0);
		if (command_buffers_allocated) pFactory->func.vkFreeCommandBuffers(pFactory->device, pFactory->cmdPool, 1, &pFactory->cmd);
		if (command_pool_created) pFactory->func.vkDestroyCommandPool(pFactory->device, pFactory->cmdPool, 0);
		if (pipeline_layout_created) pFactory->func.vkDestroyPipelineLayout(pFactory->device, pFactory->pipelineLayout, 0);
		if (descriptor_set_layout_created) pFactory->func.vkDestroyDescriptorSetLayout(pFactory->device, pFactory->descriptorSetLayout, 0);
		if (staging_buffer_and_memory_created) _destroy_buffer_and_memory(pFactory, &pFactory->staging);
		if (index_buffer_and_memory_created) _destroy_buffer_and_memory(pFactory, &pFactory->index);
	}

	return result;
}

void PixelcharImplVulkanFactoryDestroy(PixelcharImplVulkanFactory* pFactory)
{
	pFactory->func.vkUnmapMemory(pFactory->device, pFactory->staging.memory);
	pFactory->func.vkDestroyFence(pFactory->device, pFactory->fence, 0);
	pFactory->func.vkFreeCommandBuffers(pFactory->device, pFactory->cmdPool, 1, &pFactory->cmd);
	pFactory->func.vkDestroyCommandPool(pFactory->device, pFactory->cmdPool, 0);
	pFactory->func.vkDestroyPipelineLayout(pFactory->device, pFactory->pipelineLayout, 0);
	pFactory->func.vkDestroyDescriptorSetLayout(pFactory->device, pFactory->descriptorSetLayout, 0);
	_destroy_buffer_and_memory(pFactory, &pFactory->staging);
	_destroy_buffer_and_memory(pFactory, &pFactory->index);
}

VkResult PixelcharImplVulkanRendererCreate(
	PixelcharImplVulkanFactory* pFactory,
	VkRenderPass renderPass,
	uint32_t subpass,
	const uint8_t* pCustomVertexShaderSource,
	size_t customVertexShaderSourceSize,
	const uint8_t* pCustomFragmentShaderSource,
	size_t customFragmentShaderSourceSize,
	PixelcharImplVulkanRenderer* pRenderer,
	PixelcharLogCallback logCallback,
	void* logCallbackUserParam
)
{
	VkResult result = VK_SUCCESS;

	bool
		vertex_shader_module_created = false,
		fragment_shader_module_created = false,
		pipeline_created = false
	;

	pRenderer->pFactory = pFactory;

	VkShaderModule vertex_shader, fragment_shader;

	VkShaderModuleCreateInfo shader_info = { 0 };
	shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

	if (result == VK_SUCCESS)
	{
		shader_info.pCode = (uint32_t*)(pCustomVertexShaderSource == 0 ? vertex_shader_code : pCustomVertexShaderSource);
		shader_info.codeSize = (pCustomVertexShaderSource == 0 ? vertex_shader_code_len : customVertexShaderSourceSize);

		if ((result = pRenderer->pFactory->func.vkCreateShaderModule(pRenderer->pFactory->device, &shader_info, 0, &vertex_shader)) == VK_SUCCESS) vertex_shader_module_created = true;
	}
	if (result == VK_SUCCESS)
	{
		shader_info.pCode = (uint32_t*)(pCustomFragmentShaderSource == 0 ? fragment_shader_code : pCustomFragmentShaderSource);
		shader_info.codeSize = (pCustomFragmentShaderSource == 0 ? fragment_shader_code_len : customFragmentShaderSourceSize);

		if ((result = pRenderer->pFactory->func.vkCreateShaderModule(pRenderer->pFactory->device, &shader_info, 0, &fragment_shader)) == VK_SUCCESS) fragment_shader_module_created = true;
	}
	if (result == VK_SUCCESS)
	{
		VkVertexInputBindingDescription vertex_binding_description = { 0 };
		vertex_binding_description.binding = 0;
		vertex_binding_description.stride = sizeof(Pixelchar);
		vertex_binding_description.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

		VkVertexInputAttributeDescription vertex_attribute_descriptions[] =
		{
			{
				.binding = 0,
				.location = 0,
				.format = VK_FORMAT_R32G32B32A32_UINT,
				.offset = 0
			},

			{
				.binding = 0,
				.location = 1,
				.format = VK_FORMAT_R32G32B32A32_UINT,
				.offset = offsetof(Pixelchar, bitmapIndex)
			},

			{
				.binding = 0,
				.location = 2,
				.format = VK_FORMAT_R8G8B8A8_UNORM,
				.offset = offsetof(Pixelchar, color)
			},

			{
				.binding = 0,
				.location = 3,
				.format = VK_FORMAT_R8G8B8A8_UNORM,
				.offset = offsetof(Pixelchar, backgroundColor)
			},

		};


		VkPipelineVertexInputStateCreateInfo vertex_input_stage = { 0 };
		vertex_input_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_stage.vertexBindingDescriptionCount = 1;
		vertex_input_stage.pVertexBindingDescriptions = &vertex_binding_description;
		vertex_input_stage.vertexAttributeDescriptionCount = sizeof(vertex_attribute_descriptions) / sizeof(vertex_attribute_descriptions[0]);
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
		pipe_info.layout = pRenderer->pFactory->pipelineLayout;
		pipe_info.stageCount = 2;
		pipe_info.pRasterizationState = &rasterization_state;
		pipe_info.pViewportState = &viewport_state;
		pipe_info.pDynamicState = &dynamic_state;
		pipe_info.pMultisampleState = &multi_sample_state;
		pipe_info.pInputAssemblyState = &input_assembly;
		pipe_info.renderPass = renderPass;
		pipe_info.subpass = subpass;

		if ((result = pRenderer->pFactory->func.vkCreateGraphicsPipelines(pRenderer->pFactory->device, 0, 1, &pipe_info, 0, &pRenderer->pipeline)) == VK_SUCCESS) pipeline_created = true;
	}
	
	if (result != VK_SUCCESS) if (pipeline_created) pRenderer->pFactory->func.vkDestroyPipeline(pRenderer->pFactory->device, pRenderer->pipeline, 0);
	if (fragment_shader_module_created) pRenderer->pFactory->func.vkDestroyShaderModule(pRenderer->pFactory->device, fragment_shader, 0);
	if (vertex_shader_module_created) pRenderer->pFactory->func.vkDestroyShaderModule(pRenderer->pFactory->device, vertex_shader, 0);

	return result;
}

void PixelcharImplVulkanRendererDestroy(PixelcharImplVulkanRenderer* pRenderer)
{
	pRenderer->pFactory->func.vkDestroyPipeline(pRenderer->pFactory->device, pRenderer->pipeline, 0);
}

VkResult PixelcharImplVulkanFontCreate(
	PixelcharImplVulkanFactory* pFactory,
	PixelcharFont* pSourceFont,
	PixelcharImplVulkanFont* pFont,
	PixelcharLogCallback logCallback,
	void* logCallbackUserParam
)
{
	VkResult result = VK_SUCCESS;

	bool
		descriptor_pool_created = false,
		buffer_and_memory_created = false
	;

	pFont->pFactory = pFactory;

	if (result == VK_SUCCESS)
	{
		VkDescriptorPoolSize pool_size = { 0 };
		pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		pool_size.descriptorCount = 1;

		VkDescriptorPoolCreateInfo descriptor_pool_info = { 0 };
		descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptor_pool_info.maxSets = 1;
		descriptor_pool_info.poolSizeCount = 1;
		descriptor_pool_info.pPoolSizes = &pool_size;

		if ((result = pFont->pFactory->func.vkCreateDescriptorPool(pFont->pFactory->device, &descriptor_pool_info, 0, &pFont->descriptorPool)) == VK_SUCCESS) descriptor_pool_created = true;
	}
	if (result == VK_SUCCESS)
	{
		VkDescriptorSetAllocateInfo descriptor_set_info = { 0 };
		descriptor_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptor_set_info.pSetLayouts = &pFont->pFactory->descriptorSetLayout;
		descriptor_set_info.descriptorSetCount = 1;
		descriptor_set_info.descriptorPool = pFont->descriptorPool;

		result = pFont->pFactory->func.vkAllocateDescriptorSets(pFont->pFactory->device, &descriptor_set_info, &pFont->descriptorSet);
	}
	uint32_t resolution_bitmap_count[2] = { pSourceFont->resolution, pSourceFont->bitmapCount };
	if (result == VK_SUCCESS)
	{
		if ((result = _create_buffer_and_memory(
			pFont->pFactory,
			pSourceFont->bitmapCount * pSourceFont->resolution * pSourceFont->resolution / 8 + sizeof(resolution_bitmap_count),
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			&(struct memory_property_property_flags) {
			.count = 3,
				.include = { VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0 },
				.exclude = { VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 0, 0 }
			},
			&pFont->bufferAndMemory
		)) == VK_SUCCESS) buffer_and_memory_created = true;
	}
	if (result == VK_SUCCESS) result = _cmd_copy_data(pFont->pFactory, resolution_bitmap_count, sizeof(resolution_bitmap_count), pFont->bufferAndMemory.buffer, 0);
	if (result == VK_SUCCESS) result = _cmd_copy_data(pFont->pFactory, pSourceFont->pBitmaps, pFont->bufferAndMemory.bufferSize - sizeof(resolution_bitmap_count), pFont->bufferAndMemory.buffer, sizeof(resolution_bitmap_count));
	if (result == VK_SUCCESS)
	{
		VkDescriptorBufferInfo desc_buffer_info = { 0 };
		desc_buffer_info.buffer = pFont->bufferAndMemory.buffer;
		desc_buffer_info.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet buffer_write = { 0 };
		buffer_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		buffer_write.dstSet = pFont->descriptorSet;
		buffer_write.pBufferInfo = &desc_buffer_info;
		buffer_write.dstBinding = 0;
		buffer_write.descriptorCount = 1;
		buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

		pFont->pFactory->func.vkUpdateDescriptorSets(pFont->pFactory->device, 1, &buffer_write, 0, 0);
	}
	else
	{
		if (buffer_and_memory_created) _destroy_buffer_and_memory(pFont->pFactory, &pFont->bufferAndMemory);
		if (descriptor_pool_created) pFont->pFactory->func.vkDestroyDescriptorPool(pFont->pFactory->device, pFont->descriptorPool, 0);
	}

	return result;
}

void PixelcharImplVulkanFontDestroy(PixelcharImplVulkanFont* pFont)
{
	_destroy_buffer_and_memory(pFont->pFactory, &pFont->bufferAndMemory);
	pFont->pFactory->func.vkDestroyDescriptorPool(pFont->pFactory->device, pFont->descriptorPool, 0);
}

void PixelcharImplVulkanRender(
	PixelcharImplVulkanRenderer* pRenderer,
	PixelcharImplVulkanFont* pFont,
	uint32_t characterCount,
	VkBuffer vertexBuffer,
	uint32_t vertexBufferOffset,
	VkCommandBuffer commandBuffer,
	uint32_t width,
	uint32_t height,
	float shadowDevisorR,
	float shadowDevisorG,
	float shadowDevisorB,
	float shadowDevisorA
)
{
	pRenderer->pFactory->func.vkCmdBindDescriptorSets(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pRenderer->pFactory->pipelineLayout,
		0,
		1,
		&pFont->descriptorSet,
		0,
		0
	);

	pRenderer->pFactory->func.vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pRenderer->pipeline);

	pRenderer->pFactory->func.vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &vertexBufferOffset);
	pRenderer->pFactory->func.vkCmdBindIndexBuffer(commandBuffer, pRenderer->pFactory->index.buffer, 0, VK_INDEX_TYPE_UINT16);

	struct _push_constants push_constants =
	{
		.screen_size.width = width,
		.screen_size.height = height,
		.shadow_color_devisor.r = shadowDevisorR,
		.shadow_color_devisor.g = shadowDevisorG,
		.shadow_color_devisor.b = shadowDevisorB,
		.shadow_color_devisor.a = shadowDevisorA
	};

	for (uint32_t char_render_mode = 0; char_render_mode < 3; char_render_mode++)
	{
		push_constants.draw_mode = char_render_mode;

		pRenderer->pFactory->func.vkCmdPushConstants(
			commandBuffer,
			pRenderer->pFactory->pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(struct _push_constants),
			&push_constants
		);

		pRenderer->pFactory->func.vkCmdDrawIndexed(commandBuffer, 6, characterCount, 0, 0, 0);
	}

	return VK_SUCCESS;
}