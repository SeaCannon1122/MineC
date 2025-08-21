#include <pixelchar_internal.h>
#include <pixelchar/renderers/renderer_vulkan.h>

#include "vulkan_vertex_shader.h"
#include "vulkan_fragment_shader.h"

#include <stdlib.h>
#include <string.h>

#define VULKAN_INSTANCE_FUNCTION_LIST \
	VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceProperties)\
	VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceMemoryProperties)\
	VULKAN_INSTANCE_FUNCTION(vkGetDeviceProcAddr)\


#define VULKAN_DEVICE_FUNCTION_LIST \
	VULKAN_DEVICE_FUNCTION(vkDeviceWaitIdle)\
	VULKAN_DEVICE_FUNCTION(vkQueueWaitIdle)\
	VULKAN_DEVICE_FUNCTION(vkQueueSubmit)\
	VULKAN_DEVICE_FUNCTION(vkCreateBuffer)\
	VULKAN_DEVICE_FUNCTION(vkDestroyBuffer)\
	VULKAN_DEVICE_FUNCTION(vkAllocateMemory)\
	VULKAN_DEVICE_FUNCTION(vkFreeMemory)\
	VULKAN_DEVICE_FUNCTION(vkBindBufferMemory)\
	VULKAN_DEVICE_FUNCTION(vkGetBufferMemoryRequirements)\
	VULKAN_DEVICE_FUNCTION(vkMapMemory)\
	VULKAN_DEVICE_FUNCTION(vkUnmapMemory)\
	VULKAN_DEVICE_FUNCTION(vkFlushMappedMemoryRanges)\
	VULKAN_DEVICE_FUNCTION(vkCreateShaderModule)\
	VULKAN_DEVICE_FUNCTION(vkDestroyShaderModule)\
	VULKAN_DEVICE_FUNCTION(vkCreateDescriptorSetLayout)\
	VULKAN_DEVICE_FUNCTION(vkDestroyDescriptorSetLayout)\
	VULKAN_DEVICE_FUNCTION(vkCreatePipelineLayout)\
	VULKAN_DEVICE_FUNCTION(vkDestroyPipelineLayout)\
	VULKAN_DEVICE_FUNCTION(vkCreateGraphicsPipelines)\
	VULKAN_DEVICE_FUNCTION(vkDestroyPipeline)\
	VULKAN_DEVICE_FUNCTION(vkCreateCommandPool)\
	VULKAN_DEVICE_FUNCTION(vkDestroyCommandPool)\
	VULKAN_DEVICE_FUNCTION(vkAllocateCommandBuffers)\
	VULKAN_DEVICE_FUNCTION(vkFreeCommandBuffers)\
	VULKAN_DEVICE_FUNCTION(vkResetCommandBuffer)\
	VULKAN_DEVICE_FUNCTION(vkBeginCommandBuffer)\
	VULKAN_DEVICE_FUNCTION(vkEndCommandBuffer)\
	VULKAN_DEVICE_FUNCTION(vkCreateDescriptorPool)\
	VULKAN_DEVICE_FUNCTION(vkDestroyDescriptorPool)\
	VULKAN_DEVICE_FUNCTION(vkAllocateDescriptorSets)\
	VULKAN_DEVICE_FUNCTION(vkUpdateDescriptorSets)\
	VULKAN_DEVICE_FUNCTION(vkCreateFence)\
	VULKAN_DEVICE_FUNCTION(vkDestroyFence)\
	VULKAN_DEVICE_FUNCTION(vkResetFences)\
	VULKAN_DEVICE_FUNCTION(vkWaitForFences)\
	VULKAN_DEVICE_FUNCTION(vkCmdCopyBuffer)\
	VULKAN_DEVICE_FUNCTION(vkCmdPipelineBarrier)\
	VULKAN_DEVICE_FUNCTION(vkCmdBindDescriptorSets)\
	VULKAN_DEVICE_FUNCTION(vkCmdBindPipeline)\
	VULKAN_DEVICE_FUNCTION(vkCmdBindVertexBuffers)\
	VULKAN_DEVICE_FUNCTION(vkCmdBindIndexBuffer)\
	VULKAN_DEVICE_FUNCTION(vkCmdPushConstants)\
	VULKAN_DEVICE_FUNCTION(vkCmdDrawIndexed)\
	VULKAN_DEVICE_FUNCTION(vkCmdUpdateBuffer)

struct _buffer_and_memory
{
	VkBuffer buffer;
	size_t buffer_size;
	VkMemoryRequirements memory_requirements;

	VkDeviceMemory memory;
	size_t memory_size;
	VkMemoryPropertyFlags property_flags;
};

#define MEMORY_PROPERTY_REQUEST_FLAG_COUNT_MAX 4
struct memory_property_property_flags
{
	VkMemoryPropertyFlags include[MEMORY_PROPERTY_REQUEST_FLAG_COUNT_MAX];
	VkMemoryPropertyFlags exclude[MEMORY_PROPERTY_REQUEST_FLAG_COUNT_MAX];
	uint32_t count;
};

typedef struct PixelcharRendererVulkan_T
{
	struct
	{
#define VULKAN_INSTANCE_FUNCTION(name) PFN_##name name;
		VULKAN_INSTANCE_FUNCTION_LIST
#undef VULKAN_INSTANCE_FUNCTION

#define VULKAN_DEVICE_FUNCTION(name) PFN_##name name;
		VULKAN_DEVICE_FUNCTION_LIST
#undef VULKAN_DEVICE_FUNCTION
	} func;

	PixelcharFont fonts[PIXELCHAR_RENDERER_MAX_FONT_COUNT];
	struct _buffer_and_memory vulkan_fonts[PIXELCHAR_RENDERER_MAX_FONT_COUNT];

	VkInstance instance;
	VkDevice device;
	VkPhysicalDevice physical_device;
	VkQueue queue;
	uint32_t queue_index;

	VkPhysicalDeviceMemoryProperties memory_properties;
	VkPhysicalDeviceProperties device_properties;

	VkDescriptorSetLayout set_layout;
	VkDescriptorSet descriptor_set;
	VkPipelineLayout pipe_layout;
	VkDescriptorPool descriptor_pool;

	struct
	{
		VkRenderPass renderpass;
		struct
		{
			uint32_t subpass;
			VkPipeline pipeline;
		} subpasses[PIXELCHAR_RENDERER_VULKAN_MAX_SUBPASS_COUNT];
		uint32_t subpass_count;
	} renderpasses[PIXELCHAR_RENDERER_VULKAN_MAX_RENDERPASS_COUNT];
	uint32_t renderpass_count;

	struct _buffer_and_memory index;

	struct _buffer_and_memory vertex;
	struct _buffer_and_memory staging;
	void* buffer_host_handle;
	bool staging_used;

	VkFence fence;
	VkCommandPool cmd_pool;
	VkCommandBuffer cmd;

	uint32_t resource_frame_count;
	uint32_t buffer_length;
	uint32_t buffer_mext_free_indices[PIXELCHAR_RENDERER_VULKAN_MAX_RESOURCE_FRAME_COUNT];

	VkDeviceSize rendering_size;
	VkDeviceSize rendering_offset;
} PixelcharRendererVulkan_T;

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

#ifdef _MSC_VER
#define restrict __restrict
#endif

VkResult _allocate_best_memory(
	PixelcharRendererVulkan_T* restrict renderer,
	struct memory_property_property_flags* restrict property_flags,
	struct _buffer_and_memory* restrict buffer_and_memory
)
{
	VkMemoryAllocateInfo allocInfo = { 0 };
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = NULL;
	allocInfo.allocationSize = buffer_and_memory->memory_requirements.size;

	for (uint32_t pass = 0; pass < property_flags->count; pass++)
	{
		for (uint32_t i = 0; i < renderer->memory_properties.memoryTypeCount; i++ ) if (
			buffer_and_memory->memory_requirements.memoryTypeBits & (1 << i) &&
			(renderer->memory_properties.memoryTypes[i].propertyFlags & property_flags->include[pass]) == property_flags->include[pass] &&
			!(renderer->memory_properties.memoryTypes[i].propertyFlags & property_flags->exclude[pass])
		)
		{
			allocInfo.memoryTypeIndex = i;

			buffer_and_memory->memory_size = buffer_and_memory->memory_requirements.size;
			buffer_and_memory->property_flags = renderer->memory_properties.memoryTypes[i].propertyFlags;

			VkResult result = renderer->func.vkAllocateMemory(renderer->device, &allocInfo, NULL, &buffer_and_memory->memory);
			if (result == VK_SUCCESS || result == VK_ERROR_DEVICE_LOST) return result;
		}
	}
	return VK_ERROR_OUT_OF_DEVICE_MEMORY;
}

VkResult _create_buffer_and_memory(
	PixelcharRendererVulkan_T* restrict renderer,
	VkDeviceSize size,
	VkBufferUsageFlags usage_flags,
	struct memory_property_property_flags* restrict property_flags,
	struct _buffer_and_memory* restrict buffer_and_memory
)
{
	VkBufferCreateInfo buffer_info = { 0 };
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buffer_info.size = size;
	buffer_info.usage = usage_flags;

	buffer_and_memory->buffer_size = size;

	VkResult result = VK_SUCCESS;
	if ((result = renderer->func.vkCreateBuffer(renderer->device, &buffer_info, 0, &buffer_and_memory->buffer)) != VK_SUCCESS) return result;

	renderer->func.vkGetBufferMemoryRequirements(renderer->device, buffer_and_memory->buffer, &buffer_and_memory->memory_requirements);

	if ((result = _allocate_best_memory(
		renderer,
		property_flags,
		buffer_and_memory
	)) != VK_SUCCESS)
	{
		if (result == VK_ERROR_DEVICE_LOST) return VK_ERROR_DEVICE_LOST;
		if (renderer->func.vkDeviceWaitIdle(renderer->device) != VK_SUCCESS) return VK_ERROR_DEVICE_LOST;
		renderer->func.vkDestroyBuffer(renderer->device, buffer_and_memory->buffer, 0);
		return result;
	}

	if ((result = renderer->func.vkBindBufferMemory(renderer->device, buffer_and_memory->buffer, buffer_and_memory->memory, 0)) != VK_SUCCESS)
	{
		if (result == VK_ERROR_DEVICE_LOST) return VK_ERROR_DEVICE_LOST;
		if (renderer->func.vkDeviceWaitIdle(renderer->device) != VK_SUCCESS) return VK_ERROR_DEVICE_LOST;
		renderer->func.vkDestroyBuffer(renderer->device, buffer_and_memory->buffer, 0);
		renderer->func.vkFreeMemory(renderer->device, buffer_and_memory->memory, 0);
		return result;
	}

	return VK_SUCCESS;
}

void _destroy_buffer_and_memory(PixelcharRendererVulkan_T* restrict renderer, struct _buffer_and_memory* restrict buffer_and_memory)
{
	renderer->func.vkDestroyBuffer(renderer->device, buffer_and_memory->buffer, 0);
	renderer->func.vkFreeMemory(renderer->device, buffer_and_memory->memory, 0);
}

VkResult _copy_data_to_buffer(PixelcharRendererVulkan_T* renderer, void* data, size_t data_size, VkBuffer buffer)
{
	VkResult result;
	VkDeviceSize chunk_size;

	if ((result = renderer->func.vkResetCommandBuffer(renderer->cmd, 0)) != VK_SUCCESS) return result;
	if ((result = renderer->func.vkResetFences(renderer->device, 1, &renderer->fence)) != VK_SUCCESS) return result;

	VkCommandBufferBeginInfo begin_info = { 0 };
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if ((result = renderer->func.vkBeginCommandBuffer(renderer->cmd, &begin_info)) != VK_SUCCESS) return result;

	for (VkDeviceSize offset = 0; offset < data_size; offset += chunk_size)
	{
		chunk_size = (UINT16_MAX < data_size - offset ? UINT16_MAX : data_size - offset);

		renderer->func.vkCmdUpdateBuffer(renderer->cmd, buffer, offset, chunk_size, (uintptr_t)data + offset);
	}
	
	if ((result = renderer->func.vkEndCommandBuffer(renderer->cmd)) != VK_SUCCESS) return result;
	if ((result = renderer->func.vkQueueWaitIdle(renderer->queue)) != VK_SUCCESS) return result;

	VkSubmitInfo submit_info = { 0 };
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &renderer->cmd;

	if ((result = renderer->func.vkQueueSubmit(renderer->queue, 1, &submit_info, renderer->fence)) != VK_SUCCESS) return result;
	if ((result = renderer->func.vkWaitForFences(renderer->device, 1, &renderer->fence, VK_TRUE, UINT64_MAX)) != VK_SUCCESS) return result;

	return VK_SUCCESS;
}

VkResult PixelcharRendererVulkanUseFont(PixelcharRendererVulkan renderer, PixelcharFont font, uint32_t fontIndex)
{
	if (renderer->fonts[fontIndex] != NULL)
	{
		renderer->fonts[fontIndex]->reference_count--;
		if (renderer->fonts[fontIndex]->reference_count == 0) free(renderer->fonts[fontIndex]);

		if (renderer->func.vkDeviceWaitIdle(renderer->device) != VK_SUCCESS) return VK_ERROR_DEVICE_LOST;
		_destroy_buffer_and_memory(renderer, &renderer->vulkan_fonts[fontIndex]);
	}
	renderer->fonts[fontIndex] = NULL;
	if (font != NULL) 
	{
		VkResult result;

		if ((result = _create_buffer_and_memory(
			renderer,
			font->bitmaps_count * font->resolution * font->resolution / 8,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			&(struct memory_property_property_flags) {
				.count = 3,
				.include = { VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0 },
				.exclude = { VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 0, 0 }
			},
			&renderer->vulkan_fonts[fontIndex]
		)) != VK_SUCCESS) return result;

		if ((result = _copy_data_to_buffer(renderer, font->bitmaps, renderer->vulkan_fonts[fontIndex].buffer_size, renderer->vulkan_fonts[fontIndex].buffer)) != VK_SUCCESS)
		{
			if (result == VK_ERROR_DEVICE_LOST) return VK_ERROR_DEVICE_LOST;
			if (renderer->func.vkDeviceWaitIdle(renderer->device) != VK_SUCCESS) return VK_ERROR_DEVICE_LOST;
			_destroy_buffer_and_memory(renderer, &renderer->vulkan_fonts[fontIndex]);
			return result;
		}

		VkDescriptorBufferInfo desc_buffer_info = { 0 };
		desc_buffer_info.buffer = renderer->vulkan_fonts[fontIndex].buffer;
		desc_buffer_info.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet buffer_write = { 0 };
		buffer_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		buffer_write.dstSet = renderer->descriptor_set;
		buffer_write.pBufferInfo = &desc_buffer_info;
		buffer_write.dstBinding = fontIndex;
		buffer_write.descriptorCount = 1;
		buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

		renderer->func.vkUpdateDescriptorSets(renderer->device, 1, &buffer_write, 0, 0);

		renderer->fonts[fontIndex] = font;
		renderer->fonts[fontIndex]->reference_count++;
	}

	return VK_SUCCESS;
}

VkResult PixelcharRendererVulkanCreate(PixelcharRendererVulkanCreateInfo* pInfo, PixelcharRendererVulkan* pRenderer)
{
	VkResult result = VK_SUCCESS;

	bool
		index_buffer_and_memory_created = false,
		vertex_buffer_and_memory_created = false,
		staging_buffer_and_memory_created = false,
		vertex_shader_module_created = false,
		fragment_shader_module_created = false,
		descriptor_set_layout_created = false,
		pipeline_layout_created = false,
		command_pool_created = false,
		command_buffers_allocated = false,
		descriptor_pool_created = false,
		fence_created = false,
		memory_mapped = false
	;

	PixelcharRendererVulkan_T* renderer;
	
	if ((renderer = calloc(1, sizeof(PixelcharRendererVulkan_T))) == NULL) return VK_ERROR_OUT_OF_HOST_MEMORY; 

	if (result == VK_SUCCESS)
	{
		renderer->resource_frame_count = pInfo->resourceFrameCount;
		renderer->buffer_length = pInfo->maxResourceFrameCharacterCount;

		renderer->instance = pInfo->instance;
		renderer->device = pInfo->device;
		renderer->physical_device = pInfo->physicalDevice;
		renderer->queue = pInfo->queue;
		renderer->queue_index = pInfo->queueFamilyIndex;

		struct load_entry { void** load_dst; uint8_t* func_name; };

		struct load_entry instance_load_entries[] =
		{
#define VULKAN_INSTANCE_FUNCTION(name) {(void**)&renderer->func.name, #name},
			VULKAN_INSTANCE_FUNCTION_LIST
#undef VULKAN_INSTANCE_FUNCTION
		};

		struct load_entry device_load_entries[] =
		{
#define VULKAN_DEVICE_FUNCTION(name) {(void**)&renderer->func.name, #name},
			VULKAN_DEVICE_FUNCTION_LIST
#undef VULKAN_DEVICE_FUNCTION
		};

		for (uint32_t i = 0; i < sizeof(instance_load_entries) / sizeof(instance_load_entries[0]) && result == VK_SUCCESS; i++)
			if ((*instance_load_entries[i].load_dst = (void*)pInfo->pfnvkGetInstanceProcAddr(renderer->instance, instance_load_entries[i].func_name)) == NULL)
				result = VK_ERROR_UNKNOWN;

		for (uint32_t i = 0; i < sizeof(device_load_entries) / sizeof(device_load_entries[0]) && result == VK_SUCCESS; i++)
			if ((*device_load_entries[i].load_dst = (void*)renderer->func.vkGetDeviceProcAddr(renderer->device, device_load_entries[i].func_name)) == NULL) 
				result = VK_ERROR_UNKNOWN;
	}

	if (result == VK_SUCCESS)
	{
		renderer->func.vkGetPhysicalDeviceMemoryProperties(renderer->physical_device, &renderer->memory_properties);
		renderer->func.vkGetPhysicalDeviceProperties(renderer->physical_device, &renderer->device_properties);
	}

	VkBufferCreateInfo buffer_info = { 0 };
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	//index buffer
	if (result == VK_SUCCESS)
	{
		if ((result = _create_buffer_and_memory(
			renderer,
			sizeof(uint16_t) * 6,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			&(struct memory_property_property_flags) {
			.count = 3,
				.include = { VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0 },
				.exclude = { VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 0, 0 }
			},
			&renderer->index
		)) == VK_SUCCESS) index_buffer_and_memory_created = true;
	}
	//vertex buffer
	if (result == VK_SUCCESS)
	{
		if ((result = _create_buffer_and_memory(
			renderer,
			renderer->resource_frame_count * renderer->buffer_length * sizeof(Pixelchar),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			&(struct memory_property_property_flags) {
			.count = 3,
				.include = { VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0 },
				.exclude = { VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 0, 0 }
			},
			&renderer->vertex
		)) == VK_SUCCESS) vertex_buffer_and_memory_created = true;
	}

	if (result == VK_SUCCESS) if ((renderer->vertex.property_flags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) != (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
		renderer->staging_used = true;

	//staging buffer
	if (result == VK_SUCCESS) if (renderer->staging_used)
	{
		if ((result = _create_buffer_and_memory(
			renderer,
			renderer->vertex.buffer_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			&(struct memory_property_property_flags) {
				.count = 2,
				.include = { VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT },
				.exclude = { 0, 0 }
			},
			&renderer->staging
		)) == VK_SUCCESS) staging_buffer_and_memory_created = true;
	}

	VkShaderModule vertex_shader, fragment_shader;

	VkShaderModuleCreateInfo shader_info = { 0 };
	shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

	if (result == VK_SUCCESS)
	{
		shader_info.pCode = (uint32_t*)(pInfo->pCustomVertexShaderSource == 0 ? vertex_shader_code : pInfo->pCustomVertexShaderSource);
		shader_info.codeSize = (pInfo->pCustomVertexShaderSource == 0 ? vertex_shader_code_len : pInfo->customVertexShaderSourceSize);

		if ((result = renderer->func.vkCreateShaderModule(renderer->device, &shader_info, 0, &vertex_shader)) == VK_SUCCESS) vertex_shader_module_created = true;
	}
	if (result == VK_SUCCESS)
	{
		shader_info.pCode = (uint32_t*)(pInfo->pCustomFragmentShaderSource == 0 ? fragment_shader_code : pInfo->pCustomFragmentShaderSource);
		shader_info.codeSize = (pInfo->pCustomFragmentShaderSource == 0 ? fragment_shader_code_len : pInfo->customFragmentShaderSourceSize);

		if ((result = renderer->func.vkCreateShaderModule(renderer->device, &shader_info, 0, &fragment_shader)) == VK_SUCCESS) fragment_shader_module_created = true;
	}
	if (result == VK_SUCCESS)
	{
		VkDescriptorSetLayoutBinding pixelfont_buffer_binding = { 0 };
		pixelfont_buffer_binding.descriptorCount = 1;
		pixelfont_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		pixelfont_buffer_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding bindings[PIXELCHAR_RENDERER_MAX_FONT_COUNT];
		for (int32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONT_COUNT; i++)
		{
			pixelfont_buffer_binding.binding = i;
			bindings[i] = pixelfont_buffer_binding;
		}

		VkDescriptorSetLayoutCreateInfo set_layout_info = { 0 };
		set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		set_layout_info.bindingCount = PIXELCHAR_RENDERER_MAX_FONT_COUNT;
		set_layout_info.pBindings = bindings;

		if ((result = renderer->func.vkCreateDescriptorSetLayout(renderer->device, &set_layout_info, 0, &renderer->set_layout)) == VK_SUCCESS) descriptor_set_layout_created = true;
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
		pipeline_layout_info.pSetLayouts = &renderer->set_layout;
		pipeline_layout_info.pushConstantRangeCount = 1;
		pipeline_layout_info.pPushConstantRanges = &push_constant_range;

		if ((result = renderer->func.vkCreatePipelineLayout(renderer->device, &pipeline_layout_info, 0, &renderer->pipe_layout)) == VK_SUCCESS) pipeline_layout_created = true;
	}
	if (result == VK_SUCCESS)
	{
		VkVertexInputBindingDescription vertex_binding_description = { 0 };
		vertex_binding_description.binding = 0;
		vertex_binding_description.stride = sizeof(_pixelchar_renderer_char);
		vertex_binding_description.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

		VkVertexInputAttributeDescription vertex_attribute_descriptions[] =
		{
			{
				.binding = 0,
				.location = 0,
				.format = VK_FORMAT_R32_UINT,
				.offset = offsetof(_pixelchar_renderer_char, bitmapIndex)
			},

			{
				.binding = 0,
				.location = 1,
				.format = VK_FORMAT_R32_UINT,
				.offset = offsetof(_pixelchar_renderer_char, flags)
			},

			{
				.binding = 0,
				.location = 2,
				.format = VK_FORMAT_R16_UINT,
				.offset = offsetof(_pixelchar_renderer_char, fontIndex)
			},

			{
				.binding = 0,
				.location = 3,
				.format = VK_FORMAT_R16_UINT,
				.offset = offsetof(_pixelchar_renderer_char, fontResolution)
			},

			{
				.binding = 0,
				.location = 4,
				.format = VK_FORMAT_R16_UINT,
				.offset = offsetof(_pixelchar_renderer_char, scale)
			},

			{
				.binding = 0,
				.location = 5,
				.format = VK_FORMAT_R16_UINT,
				.offset = offsetof(_pixelchar_renderer_char, bitmapWidth)
			},

			{
				.binding = 0,
				.location = 6,
				.format = VK_FORMAT_R32G32_SINT,
				.offset = offsetof(_pixelchar_renderer_char, position)
			},

			{
				.binding = 0,
				.location = 7,
				.format = VK_FORMAT_R8G8B8A8_UNORM,
				.offset = offsetof(_pixelchar_renderer_char, color)
			},

			{
				.binding = 0,
				.location = 8,
				.format = VK_FORMAT_R8G8B8A8_UNORM,
				.offset = offsetof(_pixelchar_renderer_char, backgroundColor)
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
		pipe_info.layout = renderer->pipe_layout;
		pipe_info.stageCount = 2;
		pipe_info.pRasterizationState = &rasterization_state;
		pipe_info.pViewportState = &viewport_state;
		pipe_info.pDynamicState = &dynamic_state;
		pipe_info.pMultisampleState = &multi_sample_state;
		pipe_info.pInputAssemblyState = &input_assembly;

		renderer->renderpass_count = pInfo->renderPassInfoCount;
		for (uint32_t i = 0; i < renderer->renderpass_count && result == VK_SUCCESS; i++)
		{
			renderer->renderpasses[i].renderpass = pInfo->pRenderPassInfos[i].renderPass;
			renderer->renderpasses[i].subpass_count = pInfo->pRenderPassInfos[i].subPassCount;

			for (uint32_t j = 0; j < renderer->renderpasses[i].subpass_count && result == VK_SUCCESS; j++)
			{
				renderer->renderpasses[i].subpasses[j].subpass = pInfo->pRenderPassInfos[i].subPasses[j];

				pipe_info.renderPass = renderer->renderpasses[i].renderpass;
				pipe_info.subpass = renderer->renderpasses[i].subpasses[j].subpass;

				if ((result = renderer->func.vkCreateGraphicsPipelines(renderer->device, 0, 1, &pipe_info, 0, &renderer->renderpasses[i].subpasses[j].pipeline)) != VK_SUCCESS)
					renderer->renderpasses[i].subpasses[j].pipeline = VK_NULL_HANDLE;
			}

		}
	}
	if (result == VK_SUCCESS)
	{
		VkCommandPoolCreateInfo pool_info = { 0 };
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.queueFamilyIndex = renderer->queue_index;
		pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if ((result = renderer->func.vkCreateCommandPool(renderer->device, &pool_info, 0, &renderer->cmd_pool)) == VK_SUCCESS) command_pool_created = true;
	}
	if (result == VK_SUCCESS)
	{
		VkCommandBufferAllocateInfo cmd_alloc_info = { 0 };
		cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd_alloc_info.commandBufferCount = 1;
		cmd_alloc_info.commandPool = renderer->cmd_pool;
		cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		if ((result = renderer->func.vkAllocateCommandBuffers(renderer->device, &cmd_alloc_info, &renderer->cmd)) == VK_SUCCESS) command_buffers_allocated = true;
	}
	if (result == VK_SUCCESS)
	{
		VkDescriptorPoolSize pool_size = { 0 };
		pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		pool_size.descriptorCount = PIXELCHAR_RENDERER_MAX_FONT_COUNT;

		VkDescriptorPoolCreateInfo descriptor_pool_info = { 0 };
		descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptor_pool_info.maxSets = 1;
		descriptor_pool_info.poolSizeCount = 1;
		descriptor_pool_info.pPoolSizes = &pool_size;

		if ((result = renderer->func.vkCreateDescriptorPool(renderer->device, &descriptor_pool_info, 0, &renderer->descriptor_pool)) == VK_SUCCESS) descriptor_pool_created = true;
	}
	if (result == VK_SUCCESS)
	{
		VkDescriptorSetAllocateInfo descriptor_set_info = { 0 };
		descriptor_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptor_set_info.pSetLayouts = &renderer->set_layout;
		descriptor_set_info.descriptorSetCount = 1;
		descriptor_set_info.descriptorPool = renderer->descriptor_pool;

		result = renderer->func.vkAllocateDescriptorSets(renderer->device, &descriptor_set_info, &renderer->descriptor_set);
	}
	if (result == VK_SUCCESS)
	{
		VkFenceCreateInfo fence_info = { 0 };
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	
		if ((result = renderer->func.vkCreateFence(renderer->device, &fence_info, 0, &renderer->fence)) == VK_SUCCESS) fence_created = true;
	}
	if (result == VK_SUCCESS)
	{
		if ((result = renderer->func.vkMapMemory(
			renderer->device, 
			(renderer->staging_used ? renderer->staging.memory : renderer->vertex.memory),
			0,
			VK_WHOLE_SIZE, 
			0, 
			&renderer->buffer_host_handle
		)) == VK_SUCCESS) memory_mapped = true;
	}
	if (result == VK_SUCCESS)
	{
		uint16_t index_data[] = { 0, 1, 2, 1, 3, 2 };
		result = _copy_data_to_buffer(renderer, index_data, sizeof(index_data), renderer->index.buffer);
	}
	if (result == VK_ERROR_DEVICE_LOST) { free(renderer); return VK_ERROR_DEVICE_LOST; }

	if (result != VK_SUCCESS)
	{
		if (renderer->func.vkDeviceWaitIdle(renderer->device) != VK_SUCCESS) { free(renderer);  return VK_ERROR_DEVICE_LOST; }

		if (memory_mapped) renderer->func.vkUnmapMemory(renderer->device, (renderer->staging_used ? renderer->staging.memory : renderer->vertex.memory));
		if (fence_created) renderer->func.vkDestroyFence(renderer->device, renderer->fence, 0);
		if (descriptor_pool_created) renderer->func.vkDestroyDescriptorPool(renderer->device, renderer->descriptor_pool, 0);
		if (command_buffers_allocated) renderer->func.vkFreeCommandBuffers(renderer->device, renderer->cmd_pool, 1, &renderer->cmd);
		if (command_pool_created) renderer->func.vkDestroyCommandPool(renderer->device, renderer->cmd_pool, 0);

		for (uint32_t i = 0; i < PIXELCHAR_RENDERER_VULKAN_MAX_RENDERPASS_COUNT; i++)
			for (uint32_t j = 0; j < PIXELCHAR_RENDERER_VULKAN_MAX_SUBPASS_COUNT; j++)
				if (renderer->renderpasses[i].subpasses[j].pipeline != VK_NULL_HANDLE)
					renderer->func.vkDestroyPipeline(renderer->device, renderer->renderpasses[i].subpasses[j].pipeline, 0);

		if (pipeline_layout_created) renderer->func.vkDestroyPipelineLayout(renderer->device, renderer->pipe_layout, 0);
		if (descriptor_set_layout_created) renderer->func.vkDestroyDescriptorSetLayout(renderer->device, renderer->set_layout, 0);
		if (staging_buffer_and_memory_created) _destroy_buffer_and_memory(renderer, &renderer->staging);
		if (vertex_buffer_and_memory_created) _destroy_buffer_and_memory(renderer, &renderer->vertex);
		if (index_buffer_and_memory_created) _destroy_buffer_and_memory(renderer, &renderer->index);

		free(renderer);
	}
	else *pRenderer = renderer;

	if (fragment_shader_module_created) renderer->func.vkDestroyShaderModule(renderer->device, fragment_shader, 0);
	if (vertex_shader_module_created) renderer->func.vkDestroyShaderModule(renderer->device, vertex_shader, 0);

	return result;
}

void PixelcharRendererVulkanDestroy(PixelcharRendererVulkan renderer)
{
	for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONT_COUNT; i++) if (renderer->fonts[i] != NULL)
	{
		_destroy_buffer_and_memory(renderer, &renderer->vulkan_fonts[i]);
		renderer->fonts[i]->reference_count--;
		if (renderer->fonts[i]->reference_count == 0) free(renderer->fonts[i]);
	}
			

	renderer->func.vkUnmapMemory(renderer->device, (renderer->staging_used ? renderer->staging.memory : renderer->vertex.memory));
	renderer->func.vkDestroyFence(renderer->device, renderer->fence, 0);
	renderer->func.vkDestroyDescriptorPool(renderer->device, renderer->descriptor_pool, 0);
	renderer->func.vkFreeCommandBuffers(renderer->device, renderer->cmd_pool, 1, &renderer->cmd);
	renderer->func.vkDestroyCommandPool(renderer->device, renderer->cmd_pool, 0);

	for (uint32_t i = 0; i < renderer->renderpass_count; i++)
		for (uint32_t j = 0; j < renderer->renderpasses[i].subpass_count; j++)
			renderer->func.vkDestroyPipeline(renderer->device, renderer->renderpasses[i].subpasses[j].pipeline, 0);

	renderer->func.vkDestroyPipelineLayout(renderer->device, renderer->pipe_layout, 0);
	renderer->func.vkDestroyDescriptorSetLayout(renderer->device, renderer->set_layout, 0);
	if (renderer->staging_used) _destroy_buffer_and_memory(renderer, &renderer->staging);
	_destroy_buffer_and_memory(renderer, &renderer->vertex);
	_destroy_buffer_and_memory(renderer, &renderer->index);

	free(renderer);
}

VkResult PixelcharRendererVulkanUpdateRenderingData(PixelcharRendererVulkan renderer, Pixelchar* pCharacters, uint32_t characterCount, uint32_t resourceFrameIndex, VkCommandBuffer commandBuffer)
{
	uint32_t char_copy_count = (
		characterCount <= renderer->buffer_length - renderer->buffer_mext_free_indices[resourceFrameIndex] ?
		characterCount :
		renderer->buffer_length - renderer->buffer_mext_free_indices[resourceFrameIndex]
	);

	if (char_copy_count == 0) return VK_SUCCESS;

	_convert_chars_to_render_chars(pCharacters, char_copy_count, renderer->fonts);

	renderer->rendering_offset = (resourceFrameIndex * renderer->buffer_length + renderer->buffer_mext_free_indices[resourceFrameIndex]) * sizeof(Pixelchar);
	renderer->rendering_size = sizeof(Pixelchar) * char_copy_count;

	memcpy((uintptr_t)renderer->buffer_host_handle + (uintptr_t)renderer->rendering_offset, pCharacters, renderer->rendering_size);

	if (renderer->staging_used)
	{
		if (renderer->staging.property_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT == 0)
		{
			VkMappedMemoryRange mapped_memory_range = { 0 };
			mapped_memory_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			mapped_memory_range.memory = renderer->staging.memory;
			mapped_memory_range.size = renderer->rendering_size;
			mapped_memory_range.offset = renderer->rendering_offset;

			uint32_t result;
			if ((result = renderer->func.vkFlushMappedMemoryRanges(renderer->device, 1, &mapped_memory_range)) != VK_SUCCESS) return result;
		}

		VkBufferCopy copy_region = { 0 };
		copy_region.srcOffset = renderer->rendering_offset;
		copy_region.dstOffset = renderer->rendering_offset;
		copy_region.size = renderer->rendering_size;

		renderer->func.vkCmdCopyBuffer(commandBuffer, renderer->staging.buffer, renderer->vertex.buffer, 1, &copy_region);

		VkBufferMemoryBarrier barrier = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
			.buffer = renderer->vertex.buffer,
			.offset = renderer->rendering_offset,
			.size = renderer->rendering_size
		};

		renderer->func.vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
			0,
			0, NULL,
			1, &barrier,
			0, NULL
		);
	}

	renderer->buffer_mext_free_indices[resourceFrameIndex] += char_copy_count;
	return VK_SUCCESS;
}

void PixelcharRendererVulkanResetResourceFrame(PixelcharRendererVulkan renderer, uint32_t resourceFrameIndex)
{
	renderer->buffer_mext_free_indices[resourceFrameIndex] = 0;
}

void PixelcharRendererVulkanRender(
	PixelcharRendererVulkan renderer,
	VkRenderPass renderPass,
	uint32_t subPass,
	VkCommandBuffer commandBuffer,
	uint32_t width,
	uint32_t height,
	float shadowDevisorR,
	float shadowDevisorG,
	float shadowDevisorB,
	float shadowDevisorA
)
{
	VkPipeline pipeline = VK_NULL_HANDLE;
	for (uint32_t i = 0; i < renderer->renderpass_count; i++) if (renderer->renderpasses[i].renderpass == renderPass)
	{
		for (uint32_t j = 0; j < renderer->renderpasses[i].subpass_count; j++) if (renderer->renderpasses[i].subpasses[j].subpass == subPass)
		{
			pipeline = renderer->renderpasses[i].subpasses[j].pipeline;
			break;
		}
		break;
	}

	if (pipeline == VK_NULL_HANDLE) return;

	renderer->func.vkCmdBindDescriptorSets(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		renderer->pipe_layout,
		0,
		1,
		&renderer->descriptor_set,
		0,
		0
	);

	renderer->func.vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	VkDeviceSize device_size = renderer->rendering_offset;
	renderer->func.vkCmdBindVertexBuffers(commandBuffer, 0, 1, &renderer->vertex.buffer, &device_size);
	renderer->func.vkCmdBindIndexBuffer(commandBuffer, renderer->index.buffer, 0, VK_INDEX_TYPE_UINT16);

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

		renderer->func.vkCmdPushConstants(
			commandBuffer,
			renderer->pipe_layout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(struct _push_constants),
			&push_constants
		);

		renderer->func.vkCmdDrawIndexed(commandBuffer, 6, renderer->rendering_size / sizeof(Pixelchar), 0, 0, 0);
	}

	return VK_SUCCESS;
}