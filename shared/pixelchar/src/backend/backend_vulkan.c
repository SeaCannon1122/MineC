#include <pixelchar_internal.h>
#include <pixelchar/backend/backend_vulkan.h>

#include "vulkan_vertex_shader.h"
#include "vulkan_fragment_shader.h"

#include <stdlib.h>
#include <string.h>

#define STAGING_BUFFER_SIZE 65536

typedef struct _font_backend_vulkan
{
	VkBuffer buffer;
	VkDeviceMemory memory;

} _font_backend_vulkan;

typedef struct _renderer_backend_vulkan
{
	struct
	{
		PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
		PFN_vkQueueWaitIdle vkQueueWaitIdle;
		PFN_vkQueueSubmit vkQueueSubmit;
		PFN_vkCreateBuffer vkCreateBuffer;
		PFN_vkDestroyBuffer vkDestroyBuffer;
		PFN_vkAllocateMemory vkAllocateMemory;
		PFN_vkFreeMemory vkFreeMemory;
		PFN_vkBindBufferMemory vkBindBufferMemory;
		PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
		PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
		PFN_vkMapMemory vkMapMemory;
		PFN_vkUnmapMemory vkUnmapMemory;
		PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges;
		PFN_vkCreateShaderModule vkCreateShaderModule;
		PFN_vkDestroyShaderModule vkDestroyShaderModule;
		PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
		PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
		PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
		PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
		PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
		PFN_vkDestroyPipeline vkDestroyPipeline;
		PFN_vkCreateCommandPool vkCreateCommandPool;
		PFN_vkDestroyCommandPool vkDestroyCommandPool;
		PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
		PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
		PFN_vkResetCommandBuffer vkResetCommandBuffer;
		PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
		PFN_vkEndCommandBuffer vkEndCommandBuffer;
		PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
		PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
		PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
		PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
		PFN_vkCreateFence vkCreateFence;
		PFN_vkDestroyFence vkDestroyFence;
		PFN_vkResetFences vkResetFences;
		PFN_vkWaitForFences vkWaitForFences;
		PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
		PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
		PFN_vkCmdBindPipeline vkCmdBindPipeline;
		PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
		PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
		PFN_vkCmdPushConstants vkCmdPushConstants;
		PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
	} func;

	VkDevice device;
	VkPhysicalDevice physical_device;
	VkQueue queue;
	uint32_t queue_index;
	VkRenderPass render_pass;

	VkDescriptorSetLayout set_layout;
	VkDescriptorSet descriptor_set;
	VkPipelineLayout pipe_layout;
	VkPipeline pipeline;
	VkDescriptorPool descriptor_pool;

	VkBuffer vertex_index_buffer;
	VkDeviceMemory vertex_index_memory;
	VkMemoryPropertyFlags vertex_index_memory_flags;
	void* vertex_index_buffer_host_handle;

	VkBuffer staging_buffer;
	VkDeviceMemory staging_memory;
	VkMemoryPropertyFlags staging_memory_flags;
	void* staging_buffer_host_handle;
	size_t staging_buffer_size;

	VkFence fence;
	VkCommandPool cmd_pool;
	VkCommandBuffer cmd;

} _renderer_backend_vulkan;

typedef struct _push_constants
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
} _push_constants;

VkResult _allocate_best_memory(
	_renderer_backend_vulkan* backend,
	VkPhysicalDeviceMemoryProperties* memory_properties,
	VkDeviceSize size,
	uint32_t memory_type_bits,
	VkMemoryPropertyFlags* preference_flags,
	VkMemoryPropertyFlags* exclude_flags,
	uint32_t preference_count,
	VkDeviceMemory* out_memory,
	VkMemoryPropertyFlags* out_property_flags
) {
	VkResult result = VK_ERROR_OUT_OF_HOST_MEMORY;

	for (uint32_t pass = 0; pass < preference_count; pass++)
	{
		for (uint32_t i = 0; i < memory_properties->memoryTypeCount; i++)
		{
			if ((memory_type_bits & (1 << i)) != 0)
			{
				if (
					(memory_properties->memoryTypes[i].propertyFlags & preference_flags[pass]) == preference_flags[pass] &&
					!(memory_properties->memoryTypes[i].propertyFlags & exclude_flags[pass])
				)
				{
					VkMemoryAllocateInfo allocInfo = {0};
					allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
					allocInfo.pNext = NULL;
					allocInfo.allocationSize = size;
					allocInfo.memoryTypeIndex = i;

					result = backend->func.vkAllocateMemory(backend->device, &allocInfo, NULL, out_memory);

					if (result == VK_SUCCESS)
					{
						*out_property_flags = memory_properties->memoryTypes[i].propertyFlags;
						return VK_SUCCESS;
					}
				}
			}
		}
	}

	return result;
}

VkResult _pixelchar_upload_data_to_buffer(_renderer_backend_vulkan* backend, void* data, size_t data_size, VkBuffer buffer, VkDeviceSize offset, void* buffer_host_handle, VkMemoryPropertyFlags memory_flags)
{
	VkResult result;

	if ((memory_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) && (memory_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
	{
		memcpy((size_t)buffer_host_handle + offset, data, data_size);
	}
	else {
		VkDeviceSize staging_offset = 0;

		while (staging_offset < data_size)
		{
			VkDeviceSize chunk_size = (backend->staging_buffer_size < data_size - staging_offset ? backend->staging_buffer_size : data_size - staging_offset);

			memcpy(backend->staging_buffer_host_handle, (size_t)data + staging_offset, chunk_size);

			if (backend->staging_memory_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT == 0)
			{
				VkMappedMemoryRange mapped_memory_range = { 0 };
				mapped_memory_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
				mapped_memory_range.memory = backend->staging_memory;
				mapped_memory_range.offset = 0;
				mapped_memory_range.size = chunk_size;
				
				backend->func.vkFlushMappedMemoryRanges(backend->device, 1, &mapped_memory_range);
			}

			result = backend->func.vkResetCommandBuffer(backend->cmd, 0);
			if (result != VK_SUCCESS) return result;

			result = backend->func.vkResetFences(backend->device, 1, &backend->fence);
			if (result != VK_SUCCESS) return result;

			VkCommandBufferBeginInfo begin_info = { 0 };
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			result = backend->func.vkBeginCommandBuffer(backend->cmd, &begin_info);
			if (result != VK_SUCCESS) return result;

			VkBufferCopy copy_region = { 0 };
			copy_region.srcOffset = 0;
			copy_region.dstOffset = offset + staging_offset;
			copy_region.size = chunk_size;

			backend->func.vkCmdCopyBuffer(backend->cmd, backend->staging_buffer, buffer, 1, &copy_region);

			result = backend->func.vkEndCommandBuffer(backend->cmd);
			if (result != VK_SUCCESS) return result;

			result = backend->func.vkQueueWaitIdle(backend->queue);
			if (result != VK_SUCCESS) return result;

			VkSubmitInfo submit_info = { 0 };
			submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.commandBufferCount = 1;
			submit_info.pCommandBuffers = &backend->cmd;

			result = backend->func.vkQueueSubmit(backend->queue, 1, &submit_info, backend->fence);
			if (result != VK_SUCCESS) return result;

			result = backend->func.vkWaitForFences(backend->device, 1, &backend->fence, 1, 10000000000);
			if (result != VK_SUCCESS) return result;

			staging_offset += chunk_size;
		}
	}

	return VK_SUCCESS;
}

PixelcharResult _font_backend_vulkan_add_reference(PixelcharRenderer renderer, uint32_t font_index)
{
	_renderer_backend_vulkan* renderer_backend = renderer->backends[PIXELCHAR_BACKEND_VULKAN];

	if (renderer->fonts[font_index]->backends_reference_count[PIXELCHAR_BACKEND_VULKAN] == 0)
	{
		_font_backend_vulkan* font_backend = malloc(sizeof(_font_backend_vulkan));
		if (font_backend == NULL) return PIXELCHAR_ERROR_OUT_OF_MEMORY;

		memset(font_backend, 0, sizeof(_font_backend_vulkan));

		VkBufferCreateInfo buffer_info = { 0 };
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		buffer_info.size = renderer->fonts[font_index]->bitmaps_count * renderer->fonts[font_index]->resolution * renderer->fonts[font_index]->resolution / 8;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (renderer_backend->func.vkCreateBuffer(renderer_backend->device, &buffer_info, 0, &font_backend->buffer) != VK_SUCCESS)
		{
			free(font_backend);
			return PIXELCHAR_ERROR_OTHER;
		}

		VkMemoryRequirements memory_requirements;
		renderer_backend->func.vkGetBufferMemoryRequirements(renderer_backend->device, font_backend->buffer, &memory_requirements);
		VkPhysicalDeviceMemoryProperties memory_properties;
		renderer_backend->func.vkGetPhysicalDeviceMemoryProperties(renderer_backend->physical_device, &memory_properties);

		VkMemoryPropertyFlags preference_flags[] = {
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			0
		};

		VkMemoryPropertyFlags exclude_flags[] = {
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			0,
			0,
		};

		VkMemoryPropertyFlags memory_flags;
		void* buffer_host_handle = NULL;

		if (
			_allocate_best_memory(
				renderer_backend,
				&memory_properties,
				memory_requirements.size,
				memory_requirements.memoryTypeBits,
				preference_flags,
				exclude_flags,
				3,
				&font_backend->memory,
				&memory_flags
			) != VK_SUCCESS
		)
		{
			renderer_backend->func.vkDestroyBuffer(renderer_backend->device, font_backend->buffer, 0);
			free(font_backend);
			return PIXELCHAR_ERROR_OTHER;
		}

		if (renderer_backend->func.vkBindBufferMemory(renderer_backend->device, font_backend->buffer, font_backend->memory, 0) != VK_SUCCESS)
		{
			renderer_backend->func.vkDestroyBuffer(renderer_backend->device, font_backend->buffer, 0);
			renderer_backend->func.vkFreeMemory(renderer_backend->device, font_backend->memory, 0);
			free(font_backend);
			return PIXELCHAR_ERROR_OTHER;
		}

		if ((memory_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) && (memory_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
		{
			if (renderer_backend->func.vkMapMemory(renderer_backend->device, font_backend->memory, 0, buffer_info.size, 0, &buffer_host_handle) != VK_SUCCESS)
				memory_flags &= (~VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}

		if (
			_pixelchar_upload_data_to_buffer(
				renderer_backend,
				renderer->fonts[font_index]->bitmaps,
				buffer_info.size,
				font_backend->buffer,
				0,
				buffer_host_handle,
				memory_flags
			) != VK_SUCCESS
		)
		{
			renderer_backend->func.vkUnmapMemory(renderer_backend->device, font_backend->memory);
			renderer_backend->func.vkDestroyBuffer(renderer_backend->device, font_backend->buffer, 0);
			renderer_backend->func.vkFreeMemory(renderer_backend->device, font_backend->memory, 0);
			free(font_backend);
			return PIXELCHAR_ERROR_OTHER;
		}

		if ((memory_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) && (memory_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
			renderer_backend->func.vkUnmapMemory(renderer_backend->device, font_backend->memory);

		renderer->fonts[font_index]->backends[PIXELCHAR_BACKEND_VULKAN] = font_backend;
	}

	_font_backend_vulkan* font_backend = renderer->fonts[font_index]->backends[PIXELCHAR_BACKEND_VULKAN];

	VkDescriptorBufferInfo buffer_info = { 0 };
	buffer_info.buffer = font_backend->buffer;
	buffer_info.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet buffer_write = { 0 };
	buffer_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	buffer_write.dstSet = renderer_backend->descriptor_set;
	buffer_write.pBufferInfo = &buffer_info;
	buffer_write.dstBinding = font_index;
	buffer_write.descriptorCount = 1;
	buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

	renderer_backend->func.vkUpdateDescriptorSets(renderer_backend->device, 1, &buffer_write, 0, 0);

	renderer->fonts[font_index]->backends_reference_count[PIXELCHAR_BACKEND_VULKAN]++;

	return PIXELCHAR_SUCCESS;
}

void _font_backend_vulkan_sub_reference(PixelcharRenderer renderer, uint32_t font_index)
{
	_renderer_backend_vulkan* renderer_backend = renderer->backends[PIXELCHAR_BACKEND_VULKAN];
	_font_backend_vulkan* font_backend = renderer->fonts[font_index]->backends[PIXELCHAR_BACKEND_VULKAN];

	if (renderer->fonts[font_index]->backends_reference_count[PIXELCHAR_BACKEND_VULKAN] == 1)
	{
		renderer_backend->func.vkDeviceWaitIdle(renderer_backend->device);

		renderer_backend->func.vkDestroyBuffer(renderer_backend->device, font_backend->buffer, 0);
		renderer_backend->func.vkFreeMemory(renderer_backend->device, font_backend->memory, 0);

		free(font_backend);
	}

	renderer->fonts[font_index]->backends_reference_count[PIXELCHAR_BACKEND_VULKAN]--;
}

PixelcharResult pixelcharRendererBackendVulkanInitialize(
	PixelcharRenderer renderer, 
	VkDevice device,
	VkPhysicalDevice physicalDevice,
	VkQueue queue,
	uint32_t queueIndex,
	VkRenderPass renderPass,
	PFN_vkGetDeviceProcAddr pfnvkGetDeviceProcAddr,
	uint8_t* vertex_shader_custom,
	uint32_t vertex_shader_custom_length,
	uint8_t* fragment_shader_custom,
	uint32_t fragment_shader_custom_length
)
{
	if (renderer == NULL) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;
	if (renderer->backends[PIXELCHAR_BACKEND_VULKAN] != NULL) return PIXELCHAR_ERROR_BACKEND_ALREADY_INITIALIZED;

	_renderer_backend_vulkan* backend = malloc(sizeof(_renderer_backend_vulkan));
	if (backend == NULL) return PIXELCHAR_ERROR_OUT_OF_MEMORY;

	memset(backend, 0, sizeof(_renderer_backend_vulkan));

	backend->device = device;
	backend->physical_device = physicalDevice;
	backend->queue = queue;
	backend->queue_index = queueIndex;
	backend->render_pass = renderPass;

	{
		void** functions[] =
		{
			(void**)&backend->func.vkDeviceWaitIdle,
			(void**)&backend->func.vkQueueWaitIdle,
			(void**)&backend->func.vkQueueSubmit,
			(void**)&backend->func.vkCreateBuffer,
			(void**)&backend->func.vkDestroyBuffer,
			(void**)&backend->func.vkAllocateMemory,
			(void**)&backend->func.vkFreeMemory,
			(void**)&backend->func.vkBindBufferMemory,
			(void**)&backend->func.vkGetPhysicalDeviceMemoryProperties,
			(void**)&backend->func.vkGetBufferMemoryRequirements,
			(void**)&backend->func.vkMapMemory,
			(void**)&backend->func.vkUnmapMemory,
			(void**)&backend->func.vkFlushMappedMemoryRanges,
			(void**)&backend->func.vkCreateShaderModule,
			(void**)&backend->func.vkDestroyShaderModule,
			(void**)&backend->func.vkCreateDescriptorSetLayout,
			(void**)&backend->func.vkDestroyDescriptorSetLayout,
			(void**)&backend->func.vkCreatePipelineLayout,
			(void**)&backend->func.vkDestroyPipelineLayout,
			(void**)&backend->func.vkCreateGraphicsPipelines,
			(void**)&backend->func.vkDestroyPipeline,
			(void**)&backend->func.vkCreateCommandPool,
			(void**)&backend->func.vkDestroyCommandPool,
			(void**)&backend->func.vkAllocateCommandBuffers,
			(void**)&backend->func.vkFreeCommandBuffers,
			(void**)&backend->func.vkResetCommandBuffer,
			(void**)&backend->func.vkBeginCommandBuffer,
			(void**)&backend->func.vkEndCommandBuffer,
			(void**)&backend->func.vkCreateDescriptorPool,
			(void**)&backend->func.vkDestroyDescriptorPool,
			(void**)&backend->func.vkAllocateDescriptorSets,
			(void**)&backend->func.vkUpdateDescriptorSets,
			(void**)&backend->func.vkCreateFence,
			(void**)&backend->func.vkDestroyFence,
			(void**)&backend->func.vkResetFences,
			(void**)&backend->func.vkWaitForFences,
			(void**)&backend->func.vkCmdCopyBuffer,
			(void**)&backend->func.vkCmdBindDescriptorSets,
			(void**)&backend->func.vkCmdBindPipeline,
			(void**)&backend->func.vkCmdBindVertexBuffers,
			(void**)&backend->func.vkCmdBindIndexBuffer,
			(void**)&backend->func.vkCmdPushConstants,
			(void**)&backend->func.vkCmdDrawIndexed
		};

		uint8_t* function_names[] =
		{
			"vkDeviceWaitIdle",
			"vkQueueWaitIdle",
			"vkQueueSubmit",
			"vkCreateBuffer",
			"vkDestroyBuffer",
			"vkAllocateMemory",
			"vkFreeMemory",
			"vkBindBufferMemory",
			"vkGetPhysicalDeviceMemoryProperties",
			"vkGetBufferMemoryRequirements",
			"vkMapMemory",
			"vkUnmapMemory",
			"vkFlushMappedMemoryRanges",
			"vkCreateShaderModule",
			"vkDestroyShaderModule",
			"vkCreateDescriptorSetLayout",
			"vkDestroyDescriptorSetLayout",
			"vkCreatePipelineLayout",
			"vkDestroyPipelineLayout",
			"vkCreateGraphicsPipelines",
			"vkDestroyPipeline",
			"vkCreateCommandPool",
			"vkDestroyCommandPool",
			"vkAllocateCommandBuffers",
			"vkFreeCommandBuffers",
			"vkResetCommandBuffer",
			"vkBeginCommandBuffer",
			"vkEndCommandBuffer",
			"vkCreateDescriptorPool",
			"vkDestroyDescriptorPool",
			"vkAllocateDescriptorSets",
			"vkUpdateDescriptorSets",
			"vkCreateFence",
			"vkDestroyFence",
			"vkResetFences",
			"vkWaitForFences",
			"vkCmdCopyBuffer",
			"vkCmdBindDescriptorSets",
			"vkCmdBindPipeline",
			"vkCmdBindVertexBuffers",
			"vkCmdBindIndexBuffer",
			"vkCmdPushConstants",
			"vkCmdDrawIndexed"
		};

		for (uint32_t i = 0; i < sizeof(function_names) / sizeof(function_names[0]); i++)
		{
			if ((*functions[i] = (void*)pfnvkGetDeviceProcAddr(backend->device, function_names[i])) == NULL)
			{
				free(backend);
				return PIXELCHAR_ERROR_OTHER;
			}
		}
	}

	backend->func.vkDeviceWaitIdle(device);

	size_t vertex_index_buffer_size = PIXELCHAR_PAD(renderer->queue_total_length * sizeof(Pixelchar), 32) + PIXELCHAR_PAD(sizeof(uint32_t) * 6, 32);
	size_t staging_buffer_size = (STAGING_BUFFER_SIZE > renderer->queue_total_length * sizeof(Pixelchar) ? STAGING_BUFFER_SIZE : renderer->queue_total_length * sizeof(Pixelchar));

	VkBufferCreateInfo buffer_info = { 0 };
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	//vertex index buffer
	buffer_info.size = vertex_index_buffer_size;
	buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	if (backend->func.vkCreateBuffer(backend->device, &buffer_info, 0, &backend->vertex_index_buffer) != VK_SUCCESS)
	{
		free(backend);
		return PIXELCHAR_ERROR_OTHER;
	}

	//staging buffer
	buffer_info.size = staging_buffer_size;
	buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	if (backend->func.vkCreateBuffer(backend->device, &buffer_info, 0, &backend->staging_buffer) != VK_SUCCESS)
	{
		backend->func.vkDestroyBuffer(backend->device, backend->vertex_index_buffer, 0);
		free(backend);
		return PIXELCHAR_ERROR_OTHER;
	}

	
	VkPhysicalDeviceMemoryProperties memory_properties;
	backend->func.vkGetPhysicalDeviceMemoryProperties(backend->physical_device, &memory_properties);

	//vertex index memory
	VkMemoryRequirements vertex_index_memory_requirements;
	backend->func.vkGetBufferMemoryRequirements(backend->device, backend->vertex_index_buffer, &vertex_index_memory_requirements);

	VkMemoryPropertyFlags vertex_index_preference_flags[] = {
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		0
	};

	VkMemoryPropertyFlags vertex_index_exclude_flags[] = {
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		0,
		0,
	};

	if (
		_allocate_best_memory(
			backend,
			&memory_properties,
			vertex_index_memory_requirements.size,
			vertex_index_memory_requirements.memoryTypeBits,
			vertex_index_preference_flags,
			vertex_index_exclude_flags,
			3,
			&backend->vertex_index_memory,
			&backend->vertex_index_memory_flags
		) != VK_SUCCESS
	)
	{
		backend->func.vkDestroyBuffer(backend->device, backend->staging_buffer, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->vertex_index_buffer, 0);
		free(backend);
		return PIXELCHAR_ERROR_OTHER;
	}

	//staging memory
	VkMemoryRequirements staging_memory_requirements;
	backend->func.vkGetBufferMemoryRequirements(backend->device, backend->staging_buffer, &staging_memory_requirements);

	VkMemoryPropertyFlags staging_preference_flags[] = {
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
	};

	VkMemoryPropertyFlags staging_exclude_flags[] = {
		0,
		0,
	};

	if (
		_allocate_best_memory(
			backend,
			&memory_properties,
			staging_memory_requirements.size,
			staging_memory_requirements.memoryTypeBits,
			staging_preference_flags,
			staging_exclude_flags,
			4,
			&backend->staging_memory,
			&backend->staging_memory_flags
		) != VK_SUCCESS
		)
	{
		backend->func.vkFreeMemory(backend->device, backend->vertex_index_memory, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->staging_buffer, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->vertex_index_buffer, 0);
		free(backend);
		return PIXELCHAR_ERROR_OTHER;
	}

	backend->staging_buffer_size = staging_memory_requirements.size;

	if (backend->func.vkBindBufferMemory(backend->device, backend->vertex_index_buffer, backend->vertex_index_memory, 0) != VK_SUCCESS)
	{
		backend->func.vkDestroyBuffer(backend->device, backend->staging_buffer, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->vertex_index_buffer, 0);
		backend->func.vkFreeMemory(backend->device, backend->staging_memory, 0);
		backend->func.vkFreeMemory(backend->device, backend->vertex_index_memory, 0);
		free(backend);
		return PIXELCHAR_ERROR_OTHER;
	}

	if (backend->func.vkBindBufferMemory(backend->device, backend->staging_buffer, backend->staging_memory, 0) != VK_SUCCESS)
	{
		backend->func.vkDestroyBuffer(backend->device, backend->staging_buffer, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->vertex_index_buffer, 0);
		backend->func.vkFreeMemory(backend->device, backend->staging_memory, 0);
		backend->func.vkFreeMemory(backend->device, backend->vertex_index_memory, 0);
		free(backend);
		return PIXELCHAR_ERROR_OTHER;
	}

	VkShaderModule vertex_shader, fragment_shader;

	VkShaderModuleCreateInfo shader_info = { 0 };
	shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

	shader_info.pCode = (vertex_shader_custom == 0 ? vertex_shader_code : vertex_shader_custom);
	shader_info.codeSize = (vertex_shader_custom == 0 ? vertex_shader_code_len : vertex_shader_custom_length);

	if (backend->func.vkCreateShaderModule(backend->device, &shader_info, 0, &vertex_shader) != VK_SUCCESS)
	{
		backend->func.vkDestroyBuffer(backend->device, backend->staging_buffer, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->vertex_index_buffer, 0);
		backend->func.vkFreeMemory(backend->device, backend->staging_memory, 0);
		backend->func.vkFreeMemory(backend->device, backend->vertex_index_memory, 0);
		free(backend);
		return PIXELCHAR_ERROR_OTHER;
	}

	shader_info.pCode = (fragment_shader_custom == 0 ? fragment_shader_code : fragment_shader_custom);
	shader_info.codeSize = (fragment_shader_custom == 0 ? fragment_shader_code_len : fragment_shader_custom_length);

	if (backend->func.vkCreateShaderModule(backend->device, &shader_info, 0, &fragment_shader) != VK_SUCCESS)
	{
		backend->func.vkDestroyShaderModule(backend->device, vertex_shader, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->staging_buffer, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->vertex_index_buffer, 0);
		backend->func.vkFreeMemory(backend->device, backend->staging_memory, 0);
		backend->func.vkFreeMemory(backend->device, backend->vertex_index_memory, 0);
		free(backend);
		return PIXELCHAR_ERROR_OTHER;
	}

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

	if (backend->func.vkCreateDescriptorSetLayout(backend->device, &set_layout_info, 0, &backend->set_layout) != VK_SUCCESS)
	{
		backend->func.vkDestroyShaderModule(backend->device, fragment_shader, 0);
		backend->func.vkDestroyShaderModule(backend->device, vertex_shader, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->staging_buffer, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->vertex_index_buffer, 0);
		backend->func.vkFreeMemory(backend->device, backend->staging_memory, 0);
		backend->func.vkFreeMemory(backend->device, backend->vertex_index_memory, 0);
		free(backend);
		return PIXELCHAR_ERROR_OTHER;
	}

	VkPushConstantRange push_constant_range = { 0 };
	push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	push_constant_range.offset = 0;
	push_constant_range.size = sizeof(_push_constants);

	VkPipelineLayoutCreateInfo pipeline_layout_info = { 0 };
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &backend->set_layout;
	pipeline_layout_info.pushConstantRangeCount = 1;
	pipeline_layout_info.pPushConstantRanges = &push_constant_range;

	if (backend->func.vkCreatePipelineLayout(backend->device, &pipeline_layout_info, 0, &backend->pipe_layout) != VK_SUCCESS)
	{
		backend->func.vkDestroyDescriptorSetLayout(backend->device, backend->set_layout, 0);
		backend->func.vkDestroyShaderModule(backend->device, fragment_shader, 0);
		backend->func.vkDestroyShaderModule(backend->device, vertex_shader, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->staging_buffer, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->vertex_index_buffer, 0);
		backend->func.vkFreeMemory(backend->device, backend->staging_memory, 0);
		backend->func.vkFreeMemory(backend->device, backend->vertex_index_memory, 0);
		free(backend);
		return PIXELCHAR_ERROR_OTHER;
	}

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
	pipe_info.layout = backend->pipe_layout;
	pipe_info.renderPass = backend->render_pass;
	pipe_info.stageCount = 2;
	pipe_info.pRasterizationState = &rasterization_state;
	pipe_info.pViewportState = &viewport_state;
	pipe_info.pDynamicState = &dynamic_state;
	pipe_info.pMultisampleState = &multi_sample_state;
	pipe_info.pInputAssemblyState = &input_assembly;

	if (backend->func.vkCreateGraphicsPipelines(backend->device, 0, 1, &pipe_info, 0, &backend->pipeline) != VK_SUCCESS)
	{
		backend->func.vkDestroyPipelineLayout(backend->device, backend->pipe_layout, 0);
		backend->func.vkDestroyDescriptorSetLayout(backend->device, backend->set_layout, 0);
		backend->func.vkDestroyShaderModule(backend->device, fragment_shader, 0);
		backend->func.vkDestroyShaderModule(backend->device, vertex_shader, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->staging_buffer, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->vertex_index_buffer, 0);
		backend->func.vkFreeMemory(backend->device, backend->staging_memory, 0);
		backend->func.vkFreeMemory(backend->device, backend->vertex_index_memory, 0);
		free(backend);
		return PIXELCHAR_ERROR_OTHER;
	}

	backend->func.vkDestroyShaderModule(backend->device, vertex_shader, 0);
	backend->func.vkDestroyShaderModule(backend->device, fragment_shader, 0);

	VkCommandPoolCreateInfo pool_info = { 0 };
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = backend->queue_index;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (backend->func.vkCreateCommandPool(backend->device, &pool_info, 0, &backend->cmd_pool) != VK_SUCCESS)
	{
		backend->func.vkDestroyPipeline(backend->device, backend->pipeline, 0);
		backend->func.vkDestroyPipelineLayout(backend->device, backend->pipe_layout, 0);
		backend->func.vkDestroyDescriptorSetLayout(backend->device, backend->set_layout, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->staging_buffer, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->vertex_index_buffer, 0);
		backend->func.vkFreeMemory(backend->device, backend->staging_memory, 0);
		backend->func.vkFreeMemory(backend->device, backend->vertex_index_memory, 0);
		free(backend);
		return PIXELCHAR_ERROR_OTHER;
	}

	VkCommandBufferAllocateInfo cmd_alloc_info = { 0 };
	cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd_alloc_info.commandBufferCount = 1;
	cmd_alloc_info.commandPool = backend->cmd_pool;
	cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	if (backend->func.vkAllocateCommandBuffers(backend->device, &cmd_alloc_info, &backend->cmd) != VK_SUCCESS)
	{
		backend->func.vkDestroyCommandPool(backend->device, backend->cmd_pool, 0);
		backend->func.vkDestroyPipeline(backend->device, backend->pipeline, 0);
		backend->func.vkDestroyPipelineLayout(backend->device, backend->pipe_layout, 0);
		backend->func.vkDestroyDescriptorSetLayout(backend->device, backend->set_layout, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->staging_buffer, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->vertex_index_buffer, 0);
		backend->func.vkFreeMemory(backend->device, backend->staging_memory, 0);
		backend->func.vkFreeMemory(backend->device, backend->vertex_index_memory, 0);
		free(backend);
		return PIXELCHAR_ERROR_OTHER;
	}
	
	VkDescriptorPoolSize pool_size = { 0 };
	pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	pool_size.descriptorCount = PIXELCHAR_RENDERER_MAX_FONT_COUNT;

	VkDescriptorPoolCreateInfo descriptor_pool_info = { 0 };
	descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_pool_info.maxSets = 1;
	descriptor_pool_info.poolSizeCount = 1;
	descriptor_pool_info.pPoolSizes = &pool_size;

	if (backend->func.vkCreateDescriptorPool(backend->device, &descriptor_pool_info, 0, &backend->descriptor_pool) != VK_SUCCESS)
	{
		backend->func.vkFreeCommandBuffers(backend->device, backend->cmd_pool, 1, &backend->cmd);
		backend->func.vkDestroyCommandPool(backend->device, backend->cmd_pool, 0);
		backend->func.vkDestroyPipeline(backend->device, backend->pipeline, 0);
		backend->func.vkDestroyPipelineLayout(backend->device, backend->pipe_layout, 0);
		backend->func.vkDestroyDescriptorSetLayout(backend->device, backend->set_layout, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->staging_buffer, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->vertex_index_buffer, 0);
		backend->func.vkFreeMemory(backend->device, backend->staging_memory, 0);
		backend->func.vkFreeMemory(backend->device, backend->vertex_index_memory, 0);
		free(backend);
		return PIXELCHAR_ERROR_OTHER;
	}

	VkDescriptorSetAllocateInfo descriptor_set_info = { 0 };
	descriptor_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptor_set_info.pSetLayouts = &backend->set_layout;
	descriptor_set_info.descriptorSetCount = 1;
	descriptor_set_info.descriptorPool = backend->descriptor_pool;

	if (backend->func.vkAllocateDescriptorSets(backend->device, &descriptor_set_info, &backend->descriptor_set) != VK_SUCCESS)
	{
		backend->func.vkDestroyDescriptorPool(backend->device, backend->descriptor_pool, 0);
		backend->func.vkFreeCommandBuffers(backend->device, backend->cmd_pool, 1, &backend->cmd);
		backend->func.vkDestroyCommandPool(backend->device, backend->cmd_pool, 0);
		backend->func.vkDestroyPipeline(backend->device, backend->pipeline, 0);
		backend->func.vkDestroyPipelineLayout(backend->device, backend->pipe_layout, 0);
		backend->func.vkDestroyDescriptorSetLayout(backend->device, backend->set_layout, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->staging_buffer, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->vertex_index_buffer, 0);
		backend->func.vkFreeMemory(backend->device, backend->staging_memory, 0);
		backend->func.vkFreeMemory(backend->device, backend->vertex_index_memory, 0);
		free(backend);
		return PIXELCHAR_ERROR_OTHER;
	}

	VkFenceCreateInfo fence_info = { 0 };
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	
	if (backend->func.vkCreateFence(backend->device, &fence_info, 0, &backend->fence) != VK_SUCCESS)
	{
		backend->func.vkDestroyDescriptorPool(backend->device, backend->descriptor_pool, 0);
		backend->func.vkFreeCommandBuffers(backend->device, backend->cmd_pool, 1, &backend->cmd);
		backend->func.vkDestroyCommandPool(backend->device, backend->cmd_pool, 0);
		backend->func.vkDestroyPipeline(backend->device, backend->pipeline, 0);
		backend->func.vkDestroyPipelineLayout(backend->device, backend->pipe_layout, 0);
		backend->func.vkDestroyDescriptorSetLayout(backend->device, backend->set_layout, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->staging_buffer, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->vertex_index_buffer, 0);
		backend->func.vkFreeMemory(backend->device, backend->staging_memory, 0);
		backend->func.vkFreeMemory(backend->device, backend->vertex_index_memory, 0);
		free(backend);
		return PIXELCHAR_ERROR_OTHER;
	}

	if (backend->func.vkMapMemory(backend->device, backend->staging_memory, 0, staging_buffer_size, 0, &backend->staging_buffer_host_handle) != VK_SUCCESS)
	{
		backend->func.vkDestroyFence(backend->device, backend->fence, 0);
		backend->func.vkDestroyDescriptorPool(backend->device, backend->descriptor_pool, 0);
		backend->func.vkFreeCommandBuffers(backend->device, backend->cmd_pool, 1, &backend->cmd);
		backend->func.vkDestroyCommandPool(backend->device, backend->cmd_pool, 0);
		backend->func.vkDestroyPipeline(backend->device, backend->pipeline, 0);
		backend->func.vkDestroyPipelineLayout(backend->device, backend->pipe_layout, 0);
		backend->func.vkDestroyDescriptorSetLayout(backend->device, backend->set_layout, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->staging_buffer, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->vertex_index_buffer, 0);
		backend->func.vkFreeMemory(backend->device, backend->staging_memory, 0);
		backend->func.vkFreeMemory(backend->device, backend->vertex_index_memory, 0);
		free(backend);
		return PIXELCHAR_ERROR_OTHER;
	}

	if ((backend->vertex_index_memory_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) && (backend->vertex_index_memory_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
	{
		if (backend->func.vkMapMemory(backend->device, backend->vertex_index_memory, 0, vertex_index_buffer_size, 0, &backend->vertex_index_buffer_host_handle) != VK_SUCCESS)
		{
			backend->vertex_index_memory_flags &= (~VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}
	}

	uint32_t index_data[] = {0, 1, 2, 1, 3, 2};

	if (
		_pixelchar_upload_data_to_buffer(
			backend,
			index_data,
			sizeof(index_data),
			backend->vertex_index_buffer,
			PIXELCHAR_PAD(renderer->queue_total_length * sizeof(Pixelchar), 32),
			backend->vertex_index_buffer_host_handle,
			backend->vertex_index_memory_flags
		) != VK_SUCCESS
	)
	{
		if ((backend->vertex_index_memory_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) && (backend->vertex_index_memory_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
			backend->func.vkUnmapMemory(backend->device, backend->vertex_index_memory);
		backend->func.vkUnmapMemory(backend->device, backend->staging_memory);
		backend->func.vkDestroyFence(backend->device, backend->fence, 0);
		backend->func.vkDestroyDescriptorPool(backend->device, backend->descriptor_pool, 0);
		backend->func.vkFreeCommandBuffers(backend->device, backend->cmd_pool, 1, &backend->cmd);
		backend->func.vkDestroyCommandPool(backend->device, backend->cmd_pool, 0);
		backend->func.vkDestroyPipeline(backend->device, backend->pipeline, 0);
		backend->func.vkDestroyPipelineLayout(backend->device, backend->pipe_layout, 0);
		backend->func.vkDestroyDescriptorSetLayout(backend->device, backend->set_layout, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->staging_buffer, 0);
		backend->func.vkDestroyBuffer(backend->device, backend->vertex_index_buffer, 0);
		backend->func.vkFreeMemory(backend->device, backend->staging_memory, 0);
		backend->func.vkFreeMemory(backend->device, backend->vertex_index_memory, 0);
		free(backend);
		return PIXELCHAR_ERROR_OTHER;
	}

	renderer->backends[PIXELCHAR_BACKEND_VULKAN] = backend;

	for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONT_COUNT; i++)
	{
		if (renderer->fonts[i] != NULL)
		{
			if (_font_backend_vulkan_add_reference(renderer, i) == PIXELCHAR_SUCCESS)
				renderer->font_backends_referenced[i][PIXELCHAR_BACKEND_VULKAN] = true;
		}
	}

	return PIXELCHAR_SUCCESS;
}

void pixelcharRendererBackendVulkanDeinitialize(PixelcharRenderer renderer)
{
	if (renderer == NULL) return;
	if (renderer->backends[PIXELCHAR_BACKEND_VULKAN] == NULL) return;

	_renderer_backend_vulkan* backend = renderer->backends[PIXELCHAR_BACKEND_VULKAN];

	backend->func.vkDeviceWaitIdle(backend->device);

	for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONT_COUNT; i++)
	{
		if (renderer->fonts[i] != NULL && renderer->font_backends_referenced[i][PIXELCHAR_BACKEND_VULKAN] == true)
			_font_backend_vulkan_sub_reference(renderer, i);
	}

	if ((backend->vertex_index_memory_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) && (backend->vertex_index_memory_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
		backend->func.vkUnmapMemory(backend->device, backend->vertex_index_memory);
	backend->func.vkUnmapMemory(backend->device, backend->staging_memory);
	backend->func.vkDestroyFence(backend->device, backend->fence, 0);
	backend->func.vkDestroyDescriptorPool(backend->device, backend->descriptor_pool, 0);
	backend->func.vkFreeCommandBuffers(backend->device, backend->cmd_pool, 1, &backend->cmd);
	backend->func.vkDestroyCommandPool(backend->device, backend->cmd_pool, 0);
	backend->func.vkDestroyPipeline(backend->device, backend->pipeline, 0);
	backend->func.vkDestroyPipelineLayout(backend->device, backend->pipe_layout, 0);
	backend->func.vkDestroyDescriptorSetLayout(backend->device, backend->set_layout, 0);
	backend->func.vkDestroyBuffer(backend->device, backend->staging_buffer, 0);
	backend->func.vkDestroyBuffer(backend->device, backend->vertex_index_buffer, 0);
	backend->func.vkFreeMemory(backend->device, backend->staging_memory, 0);
	backend->func.vkFreeMemory(backend->device, backend->vertex_index_memory, 0);

	free(backend);
	renderer->backends[PIXELCHAR_BACKEND_VULKAN] = NULL;
}

PixelcharResult pixelcharRendererBackendVulkanUpdateRenderingData(PixelcharRenderer renderer, VkCommandBuffer commandBuffer)
{
	if (renderer == NULL) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;

	if (renderer->queue_filled_length == 0) return;

	if (renderer->backends[PIXELCHAR_BACKEND_VULKAN] == NULL) return PIXELCHAR_ERROR_BACKEND_NOT_INITIALIZED;

	_renderer_backend_vulkan* backend = renderer->backends[PIXELCHAR_BACKEND_VULKAN];

	_pixelchar_renderer_convert_queue(renderer, PIXELCHAR_BACKEND_VULKAN);

	if ((backend->vertex_index_memory_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) && (backend->vertex_index_memory_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
	{
		memcpy(backend->vertex_index_buffer_host_handle, renderer->queue, sizeof(Pixelchar) * renderer->queue_filled_length);
	}
	else {

		memcpy(backend->staging_buffer_host_handle, renderer->queue, sizeof(Pixelchar) * renderer->queue_filled_length);

		VkBufferCopy copy_region = { 0 };
		copy_region.srcOffset = 0;
		copy_region.dstOffset = 0;
		copy_region.size = sizeof(Pixelchar) * renderer->queue_filled_length;

		backend->func.vkCmdCopyBuffer(commandBuffer, backend->staging_buffer, backend->vertex_index_buffer, 1, &copy_region);

		VkBufferMemoryBarrier barrier = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.buffer = backend->vertex_index_buffer,
			.offset = 0,
			.size = VK_WHOLE_SIZE,
		};

		/*vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
			0,
			0, NULL,
			1, &barrier,
			0, NULL
		);*/
	}
}

PixelcharResult pixelcharRendererBackendVulkanRender(
	PixelcharRenderer renderer,
	VkCommandBuffer commandBuffer,
	uint32_t width,
	uint32_t height,
	float shadowDevisorR,
	float shadowDevisorG,
	float shadowDevisorB,
	float shadowDevisorA
)
{
	if (renderer == NULL) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;
	if (width == 0) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;
	if (height == 0) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;

	if (renderer->queue_filled_length == 0) return;

	if (renderer->backends[PIXELCHAR_BACKEND_VULKAN] == NULL) return PIXELCHAR_ERROR_BACKEND_NOT_INITIALIZED;

	_renderer_backend_vulkan* backend = renderer->backends[PIXELCHAR_BACKEND_VULKAN];

	backend->func.vkCmdBindDescriptorSets(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		backend->pipe_layout,
		0,
		1,
		&backend->descriptor_set,
		0,
		0
	);

	backend->func.vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, backend->pipeline);

	VkDeviceSize device_size = 0;
	backend->func.vkCmdBindVertexBuffers(commandBuffer, 0, 1, &backend->vertex_index_buffer, &device_size);
	backend->func.vkCmdBindIndexBuffer(commandBuffer, backend->vertex_index_buffer, PIXELCHAR_PAD(renderer->queue_total_length * sizeof(Pixelchar), 32), VK_INDEX_TYPE_UINT32);

	_push_constants push_constants;
	push_constants.screen_size.width = width;
	push_constants.screen_size.height = height;
	push_constants.shadow_color_devisor.r = shadowDevisorR;
	push_constants.shadow_color_devisor.g = shadowDevisorG;
	push_constants.shadow_color_devisor.b = shadowDevisorB;
	push_constants.shadow_color_devisor.a = shadowDevisorA;

	for (uint32_t char_render_mode = 0; char_render_mode < 3; char_render_mode++)
	{
		push_constants.draw_mode = char_render_mode;

		backend->func.vkCmdPushConstants(
			commandBuffer,
			backend->pipe_layout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(_push_constants),
			&push_constants
		);

		backend->func.vkCmdDrawIndexed(commandBuffer, 6, renderer->queue_filled_length, 0, 0, 0);
	}

	renderer->queue_filled_length = 0;
}