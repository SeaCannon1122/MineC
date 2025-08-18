#include <pixelchar_internal.h>
#include <pixelchar/backend/backend_vulkan.h>

#include "vulkan_vertex_shader.h"
#include "vulkan_fragment_shader.h"

#include <stdlib.h>
#include <string.h>

#define STAGING_BUFFER_SIZE 65536

#define VULKAN_INSTANCE_FUNCTION_LIST \
	VULKAN_INSTANCE_FUNCTION(PFN_vkGetPhysicalDeviceMemoryProperties, vkGetPhysicalDeviceMemoryProperties)\
	VULKAN_INSTANCE_FUNCTION(PFN_vkGetDeviceProcAddr, vkGetDeviceProcAddr)\


#define VULKAN_DEVICE_FUNCTION_LIST \
	VULKAN_DEVICE_FUNCTION(PFN_vkDeviceWaitIdle, vkDeviceWaitIdle)\
	VULKAN_DEVICE_FUNCTION(PFN_vkQueueWaitIdle, vkQueueWaitIdle)\
	VULKAN_DEVICE_FUNCTION(PFN_vkQueueSubmit, vkQueueSubmit)\
	VULKAN_DEVICE_FUNCTION(PFN_vkCreateBuffer, vkCreateBuffer)\
	VULKAN_DEVICE_FUNCTION(PFN_vkDestroyBuffer, vkDestroyBuffer)\
	VULKAN_DEVICE_FUNCTION(PFN_vkAllocateMemory, vkAllocateMemory)\
	VULKAN_DEVICE_FUNCTION(PFN_vkFreeMemory, vkFreeMemory)\
	VULKAN_DEVICE_FUNCTION(PFN_vkBindBufferMemory, vkBindBufferMemory)\
	VULKAN_DEVICE_FUNCTION(PFN_vkGetBufferMemoryRequirements, vkGetBufferMemoryRequirements)\
	VULKAN_DEVICE_FUNCTION(PFN_vkMapMemory, vkMapMemory)\
	VULKAN_DEVICE_FUNCTION(PFN_vkUnmapMemory, vkUnmapMemory)\
	VULKAN_DEVICE_FUNCTION(PFN_vkFlushMappedMemoryRanges, vkFlushMappedMemoryRanges)\
	VULKAN_DEVICE_FUNCTION(PFN_vkCreateShaderModule, vkCreateShaderModule)\
	VULKAN_DEVICE_FUNCTION(PFN_vkDestroyShaderModule, vkDestroyShaderModule)\
	VULKAN_DEVICE_FUNCTION(PFN_vkCreateDescriptorSetLayout, vkCreateDescriptorSetLayout)\
	VULKAN_DEVICE_FUNCTION(PFN_vkDestroyDescriptorSetLayout, vkDestroyDescriptorSetLayout)\
	VULKAN_DEVICE_FUNCTION(PFN_vkCreatePipelineLayout, vkCreatePipelineLayout)\
	VULKAN_DEVICE_FUNCTION(PFN_vkDestroyPipelineLayout, vkDestroyPipelineLayout)\
	VULKAN_DEVICE_FUNCTION(PFN_vkCreateGraphicsPipelines, vkCreateGraphicsPipelines)\
	VULKAN_DEVICE_FUNCTION(PFN_vkDestroyPipeline, vkDestroyPipeline)\
	VULKAN_DEVICE_FUNCTION(PFN_vkCreateCommandPool, vkCreateCommandPool)\
	VULKAN_DEVICE_FUNCTION(PFN_vkDestroyCommandPool, vkDestroyCommandPool)\
	VULKAN_DEVICE_FUNCTION(PFN_vkAllocateCommandBuffers, vkAllocateCommandBuffers)\
	VULKAN_DEVICE_FUNCTION(PFN_vkFreeCommandBuffers, vkFreeCommandBuffers)\
	VULKAN_DEVICE_FUNCTION(PFN_vkResetCommandBuffer, vkResetCommandBuffer)\
	VULKAN_DEVICE_FUNCTION(PFN_vkBeginCommandBuffer, vkBeginCommandBuffer)\
	VULKAN_DEVICE_FUNCTION(PFN_vkEndCommandBuffer, vkEndCommandBuffer)\
	VULKAN_DEVICE_FUNCTION(PFN_vkCreateDescriptorPool, vkCreateDescriptorPool)\
	VULKAN_DEVICE_FUNCTION(PFN_vkDestroyDescriptorPool, vkDestroyDescriptorPool)\
	VULKAN_DEVICE_FUNCTION(PFN_vkAllocateDescriptorSets, vkAllocateDescriptorSets)\
	VULKAN_DEVICE_FUNCTION(PFN_vkUpdateDescriptorSets, vkUpdateDescriptorSets)\
	VULKAN_DEVICE_FUNCTION(PFN_vkCreateFence, vkCreateFence)\
	VULKAN_DEVICE_FUNCTION(PFN_vkDestroyFence, vkDestroyFence)\
	VULKAN_DEVICE_FUNCTION(PFN_vkResetFences, vkResetFences)\
	VULKAN_DEVICE_FUNCTION(PFN_vkWaitForFences, vkWaitForFences)\
	VULKAN_DEVICE_FUNCTION(PFN_vkCmdCopyBuffer, vkCmdCopyBuffer)\
	VULKAN_DEVICE_FUNCTION(PFN_vkCmdBindDescriptorSets, vkCmdBindDescriptorSets)\
	VULKAN_DEVICE_FUNCTION(PFN_vkCmdBindPipeline, vkCmdBindPipeline)\
	VULKAN_DEVICE_FUNCTION(PFN_vkCmdBindVertexBuffers, vkCmdBindVertexBuffers)\
	VULKAN_DEVICE_FUNCTION(PFN_vkCmdBindIndexBuffer, vkCmdBindIndexBuffer)\
	VULKAN_DEVICE_FUNCTION(PFN_vkCmdPushConstants, vkCmdPushConstants)\
	VULKAN_DEVICE_FUNCTION(PFN_vkCmdDrawIndexed, vkCmdDrawIndexed)\

struct _memory
{
	VkDeviceMemory memory;
	size_t size;
	VkMemoryPropertyFlags property_flags;
	void* host_handle;
};

#define MEMORY_PROPERTY_REQUEST_FLAG_COUNT_MAX 4
struct memory_property_request_flags
{
	VkMemoryPropertyFlags include[MEMORY_PROPERTY_REQUEST_FLAG_COUNT_MAX];
	VkMemoryPropertyFlags exclude[MEMORY_PROPERTY_REQUEST_FLAG_COUNT_MAX];
	uint32_t count;
};

typedef struct _font_backend_vulkan
{
	VkBuffer buffer;
	struct _memory memory;

} _font_backend_vulkan;

typedef struct _renderer_backend_vulkan
{
	struct
	{
#define VULKAN_INSTANCE_FUNCTION(signature, name) signature name;
		VULKAN_INSTANCE_FUNCTION_LIST
#undef VULKAN_INSTANCE_FUNCTION

#define VULKAN_DEVICE_FUNCTION(signature, name) signature name;
		VULKAN_DEVICE_FUNCTION_LIST
#undef VULKAN_DEVICE_FUNCTION
	} func;

	VkInstance instance;
	VkDevice device;
	VkPhysicalDevice physical_device;
	VkQueue queue;
	uint32_t queue_index;
	VkRenderPass render_pass;
	uint32_t subpass;

	VkPhysicalDeviceMemoryProperties memory_properties;

	VkDescriptorSetLayout set_layout;
	VkDescriptorSet descriptor_set;
	VkPipelineLayout pipe_layout;
	VkPipeline pipeline;
	VkDescriptorPool descriptor_pool;

	uint32_t resource_frame_count;

	VkBuffer index_buffer;
	VkDeviceMemory index_buffer_memory;

	VkBuffer vertex_index_buffer;
	struct _memory vertex_index_memory;

	VkBuffer staging_buffer;
	struct _memory staging_memory;

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

#ifdef _MSC_VER
#define restrict __restrict
#endif

bool _find_suitable_memory_type(
	uint32_t* restrict type,
	VkPhysicalDeviceMemoryProperties* restrict memory_props,
	uint32_t memory_type_bits,
	VkMemoryPropertyFlags include_flags,
	VkMemoryPropertyFlags exclude_flags
)
{
	for (; *type < memory_props->memoryTypeCount; (*type)++)
	{
		if (
			memory_type_bits & (1 << *type) &&
			(memory_props->memoryTypes[*type].propertyFlags & include_flags) == include_flags &&
			!(memory_props->memoryTypes[*type].propertyFlags & exclude_flags)
		) return true;
	}
	return false;
}

struct _memory _allocate_best_memory(
	_renderer_backend_vulkan* restrict backend,
	VkDeviceSize size,
	uint32_t memory_type_bits,
	struct memory_property_request_flags* restrict request_flags
)
{
	VkMemoryAllocateInfo allocInfo = { 0 };
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = NULL;
	allocInfo.allocationSize = size;

	for (uint32_t pass = 0; pass <= request_flags->count; pass++)
	{
		uint32_t i = 0;
		while (_find_suitable_memory_type(
			&i, 
			&backend->memory_properties, 
			memory_type_bits, 
			(pass < request_flags->count ? request_flags->include[pass] : 0),
			(pass < request_flags->count ? request_flags->exclude[pass] : 0)
		))
		{
			allocInfo.memoryTypeIndex = i;

			struct _memory memory = {.size = size, .property_flags = backend->memory_properties.memoryTypes[i].propertyFlags};
			if (backend->func.vkAllocateMemory(backend->device, &allocInfo, NULL, &memory.memory) == VK_SUCCESS) return memory;
		}
	}
	return (struct _memory){.memory = NULL};
}

VkResult _pixelchar_upload_data_to_buffer(_renderer_backend_vulkan* backend, void* data, size_t data_size, VkBuffer buffer, VkDeviceSize offset, void* buffer_host_handle, VkMemoryPropertyFlags memory_flags)
{
	VkResult result;

	if ((memory_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) && (memory_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
	{
		memcpy((void*)((size_t)buffer_host_handle + offset), data, data_size);
	}
	else {
		VkDeviceSize staging_offset = 0;

		while (staging_offset < data_size)
		{
			VkDeviceSize chunk_size = (backend->staging_memory.size < data_size - staging_offset ? backend->staging_memory.size : data_size - staging_offset);

			memcpy(backend->staging_memory.host_handle, (const void*)((size_t)data + staging_offset), chunk_size);

			if (backend->staging_memory.property_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT == 0)
			{
				VkMappedMemoryRange mapped_memory_range = { 0 };
				mapped_memory_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
				mapped_memory_range.memory = backend->staging_memory.memory;
				mapped_memory_range.offset = 0;
				mapped_memory_range.size = chunk_size;
				
				if ((result = backend->func.vkFlushMappedMemoryRanges(backend->device, 1, &mapped_memory_range)) != VK_SUCCESS) return result;
			}

			if ((result = backend->func.vkResetCommandBuffer(backend->cmd, 0)) != VK_SUCCESS) return result;
			if ((result = backend->func.vkResetFences(backend->device, 1, &backend->fence)) != VK_SUCCESS) return result;

			VkCommandBufferBeginInfo begin_info = { 0 };
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			if ((result = backend->func.vkBeginCommandBuffer(backend->cmd, &begin_info)) != VK_SUCCESS) return result;

			VkBufferCopy copy_region = { 0 };
			copy_region.srcOffset = 0;
			copy_region.dstOffset = offset + staging_offset;
			copy_region.size = chunk_size;

			backend->func.vkCmdCopyBuffer(backend->cmd, backend->staging_buffer, buffer, 1, &copy_region);

			if ((result = backend->func.vkEndCommandBuffer(backend->cmd)) != VK_SUCCESS) return result;
			if ((result = backend->func.vkQueueWaitIdle(backend->queue)) != VK_SUCCESS) return result;

			VkSubmitInfo submit_info = { 0 };
			submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.commandBufferCount = 1;
			submit_info.pCommandBuffers = &backend->cmd;

			if ((result = backend->func.vkQueueSubmit(backend->queue, 1, &submit_info, backend->fence)) != VK_SUCCESS) return result;
			if ((result = backend->func.vkWaitForFences(backend->device, 1, &backend->fence, 1, 10000000000)) != VK_SUCCESS) return result;

			staging_offset += chunk_size;
		}
	}

	return VK_SUCCESS;
}

PixelcharResult _font_backend_vulkan_add_reference(PixelcharRenderer renderer, uint32_t font_index, uint32_t backend_slot_index)
{
	_renderer_backend_vulkan* renderer_backend = renderer->backends[backend_slot_index].data;

	if (renderer->fonts[font_index]->backends_reference_count[backend_slot_index] == 0)
	{
		_font_backend_vulkan* font_backend = calloc(1, sizeof(_font_backend_vulkan));
		if (font_backend == NULL) return PIXELCHAR_ERROR_OUT_OF_MEMORY;

		VkBufferCreateInfo buffer_info = { 0 };
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		buffer_info.size = renderer->fonts[font_index]->bitmaps_count * renderer->fonts[font_index]->resolution * renderer->fonts[font_index]->resolution / 8;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (renderer_backend->func.vkCreateBuffer(renderer_backend->device, &buffer_info, 0, &font_backend->buffer) != VK_SUCCESS)
		{
			free(font_backend);
			return PIXELCHAR_ERROR_BACKEND_API;
		}

		VkMemoryRequirements memory_requirements;
		renderer_backend->func.vkGetBufferMemoryRequirements(renderer_backend->device, font_backend->buffer, &memory_requirements);

		VkMemoryPropertyFlags memory_flags;
		void* buffer_host_handle = NULL;

		if (
			(font_backend->memory = _allocate_best_memory(
				renderer_backend,
				memory_requirements.size,
				memory_requirements.memoryTypeBits,
				&(struct memory_property_request_flags) {
					.count = 3, 
					.include = { VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT },
					.exclude = { VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 0}
				}
			)).memory
		)
		{
			renderer_backend->func.vkDestroyBuffer(renderer_backend->device, font_backend->buffer, 0);
			free(font_backend);
			return PIXELCHAR_ERROR_BACKEND_API;
		}

		if (renderer_backend->func.vkBindBufferMemory(renderer_backend->device, font_backend->buffer, font_backend->memory, 0) != VK_SUCCESS)
		{
			renderer_backend->func.vkDestroyBuffer(renderer_backend->device, font_backend->buffer, 0);
			renderer_backend->func.vkFreeMemory(renderer_backend->device, font_backend->memory, 0);
			free(font_backend);
			return PIXELCHAR_ERROR_BACKEND_API;
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
			return PIXELCHAR_ERROR_BACKEND_API;
		}

		if ((memory_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) && (memory_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
			renderer_backend->func.vkUnmapMemory(renderer_backend->device, font_backend->memory);

		renderer->fonts[font_index]->backends[backend_slot_index] = font_backend;
	}

	_font_backend_vulkan* font_backend = renderer->fonts[font_index]->backends[backend_slot_index];

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

	renderer->fonts[font_index]->backends_reference_count[backend_slot_index]++;

	return PIXELCHAR_SUCCESS;
}

void _font_backend_vulkan_sub_reference(PixelcharRenderer renderer, uint32_t font_index, uint32_t backend_slot_index)
{
	_renderer_backend_vulkan* renderer_backend = renderer->backends[backend_slot_index].data;
	_font_backend_vulkan* font_backend = renderer->fonts[font_index]->backends[backend_slot_index];

	if (renderer->fonts[font_index]->backends_reference_count[backend_slot_index] == 1)
	{
		if (renderer_backend->func.vkDeviceWaitIdle(renderer_backend->device) == VK_SUCCESS)
		{
			renderer_backend->func.vkDestroyBuffer(renderer_backend->device, font_backend->buffer, 0);
			renderer_backend->func.vkFreeMemory(renderer_backend->device, font_backend->memory, 0);
		}
		free(font_backend);
	}

	renderer->fonts[font_index]->backends_reference_count[backend_slot_index]--;
}

PixelcharResult pixelcharRendererBackendVulkanInitialize(
	PixelcharRenderer renderer,
	uint32_t backendSlotIndex,
	uint32_t resourceFrameCount,
	VkInstance instance,
	VkDevice device,
	VkPhysicalDevice physicalDevice,
	VkQueue transferQueue,
	uint32_t transferQueueFamilyIndex,
	VkRenderPass renderPass,
	uint32_t subpass,
	PFN_vkGetInstanceProcAddr pfnvkGetInstanceProcAddr,
	const uint8_t* customVertexShaderSource,
	size_t customVertexShaderSourceSize,
	const uint8_t* customFragmentShaderSource,
	size_t customFragmentShaderSourceSize
)
{
	if (renderer == NULL) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;
	if (backendSlotIndex >= PIXELCHAR_RENDERER_MAX_BACKEND_COUNT) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;
	if (pfnvkGetInstanceProcAddr == NULL) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;
	if (renderer->backends[backendSlotIndex].data != NULL) return PIXELCHAR_ERROR_BACKEND_SLOT_ALREADY_IN_USED;

	PixelcharResult result = PIXELCHAR_SUCCESS;

	bool
		backend_memory_allocated = false,
		vertex_index_buffer_created = false,
		staging_buffer_created = false,
		vertex_index_memory_allocated = false,
		staging_memory_allocated = false,
		vertex_shader_module_created = false,
		fragment_shader_module_created = false,
		descriptor_set_layout_created = false,
		pipeline_layout_created = false,
		pipeline_created = false,
		command_pool_created = false,
		command_buffers_allocated = false,
		descriptor_pool_created = false,
		fence_created = false,
		staging_memory_mapped = false,
		vertex_index_memory_mapped = false
	;

	_renderer_backend_vulkan* backend; 
	
	if ((backend = calloc(1, sizeof(_renderer_backend_vulkan))) == NULL) result = PIXELCHAR_ERROR_OUT_OF_MEMORY;
	else backend_memory_allocated = true;

	if (result == PIXELCHAR_SUCCESS)
	{
		backend->device = device;
		backend->physical_device = physicalDevice;
		backend->queue = transferQueue;
		backend->queue_index = transferQueueFamilyIndex;
		backend->render_pass = renderPass;
		backend->subpass = subpass;

		struct load_entry { void** load_dst; uint8_t* func_name; };
#define LOAD_FUNC_ENTRY(func_name) 

		struct load_entry instance_load_entries[] =
		{
#define VULKAN_INSTANCE_FUNCTION(signature, name) {(void**)&backend->func.name, #name},
			VULKAN_INSTANCE_FUNCTION_LIST
#undef VULKAN_INSTANCE_FUNCTION
		};

		struct load_entry device_load_entries[] =
		{
#define VULKAN_DEVICE_FUNCTION(signature, name) {(void**)&backend->func.name, #name},
			VULKAN_DEVICE_FUNCTION_LIST
#undef VULKAN_DEVICE_FUNCTION
		};

		for (uint32_t i = 0; i < sizeof(instance_load_entries) / sizeof(instance_load_entries[0]) && result == PIXELCHAR_SUCCESS; i++)
			if ((*instance_load_entries[i].load_dst = (void*)pfnvkGetInstanceProcAddr(instance, instance_load_entries[i].func_name)) == NULL)
				result = PIXELCHAR_ERROR_BACKEND_API;

		for (uint32_t i = 0; i < sizeof(device_load_entries) / sizeof(device_load_entries[0]) && result == PIXELCHAR_SUCCESS; i++)
			if ((*device_load_entries[i].load_dst = (void*)backend->func.vkGetDeviceProcAddr(backend->device, device_load_entries[i].func_name)) == NULL) 
				result = PIXELCHAR_ERROR_BACKEND_API;
	}

	if (result == PIXELCHAR_SUCCESS) backend->func.vkGetPhysicalDeviceMemoryProperties(backend->physical_device, &backend->memory_properties);

	size_t vertex_index_buffer_size = PIXELCHAR_PAD(renderer->queue_total_length * sizeof(Pixelchar), 32) + PIXELCHAR_PAD(sizeof(uint32_t) * 6, 32);
	size_t staging_buffer_size = (STAGING_BUFFER_SIZE > renderer->queue_total_length * sizeof(Pixelchar) ? STAGING_BUFFER_SIZE : renderer->queue_total_length * sizeof(Pixelchar));

	VkBufferCreateInfo buffer_info = { 0 };
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	//vertex index buffer
	buffer_info.size = vertex_index_buffer_size;
	buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	if (result == PIXELCHAR_SUCCESS)
	{
		backend->func.vkDeviceWaitIdle(device);

		if (backend->func.vkCreateBuffer(backend->device, &buffer_info, 0, &backend->vertex_index_buffer) != VK_SUCCESS) result = PIXELCHAR_ERROR_BACKEND_API;
		else vertex_index_buffer_created = true;
	}
	//staging buffer
	buffer_info.size = staging_buffer_size;
	buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	if (result == PIXELCHAR_SUCCESS)
	{
		if (backend->func.vkCreateBuffer(backend->device, &buffer_info, 0, &backend->staging_buffer) != VK_SUCCESS) result = PIXELCHAR_ERROR_BACKEND_API;
		else staging_buffer_created = true;
	}

	//vertex index memory
	VkMemoryRequirements vertex_index_memory_requirements;
	if (result == PIXELCHAR_SUCCESS) backend->func.vkGetBufferMemoryRequirements(backend->device, backend->vertex_index_buffer, &vertex_index_memory_requirements);

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
	if (result == PIXELCHAR_SUCCESS)
	{
		if (
			_allocate_best_memory(
				backend,
				vertex_index_memory_requirements.size,
				vertex_index_memory_requirements.memoryTypeBits,
				vertex_index_preference_flags,
				vertex_index_exclude_flags,
				3,
				&backend->vertex_index_memory,
				&backend->vertex_index_memory_flags
			) != VK_SUCCESS
		) result = PIXELCHAR_ERROR_BACKEND_API;
		else vertex_index_memory_allocated = true;
	}

	//staging memory
	VkMemoryRequirements staging_memory_requirements;
	if (result == PIXELCHAR_SUCCESS) backend->func.vkGetBufferMemoryRequirements(backend->device, backend->staging_buffer, &staging_memory_requirements);

	VkMemoryPropertyFlags staging_preference_flags[] = {
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
	};

	VkMemoryPropertyFlags staging_exclude_flags[] = {
		0,
		0,
	};

	if (result == PIXELCHAR_SUCCESS)
	{
		if (
			_allocate_best_memory(
				backend,
				staging_memory_requirements.size,
				staging_memory_requirements.memoryTypeBits,
				staging_preference_flags,
				staging_exclude_flags,
				4,
				&backend->staging_memory,
				&backend->staging_memory_flags
			) != VK_SUCCESS
		) result = PIXELCHAR_ERROR_BACKEND_API;
		else staging_memory_allocated = true;
	}

	if (result == PIXELCHAR_SUCCESS) backend->staging_buffer_size = staging_memory_requirements.size;

	if (result == PIXELCHAR_SUCCESS) if (backend->func.vkBindBufferMemory(backend->device, backend->vertex_index_buffer, backend->vertex_index_memory, 0) != VK_SUCCESS) result = PIXELCHAR_ERROR_BACKEND_API;
	if (result == PIXELCHAR_SUCCESS) if (backend->func.vkBindBufferMemory(backend->device, backend->staging_buffer, backend->staging_memory, 0) != VK_SUCCESS) result = PIXELCHAR_ERROR_BACKEND_API;

	VkShaderModule vertex_shader, fragment_shader;

	VkShaderModuleCreateInfo shader_info = { 0 };
	shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

	shader_info.pCode = (uint32_t*)(customVertexShaderSource == 0 ? vertex_shader_code : customVertexShaderSource);
	shader_info.codeSize = (customVertexShaderSource == 0 ? vertex_shader_code_len : customVertexShaderSourceSize);

	if (result == PIXELCHAR_SUCCESS)
	{
		if (backend->func.vkCreateShaderModule(backend->device, &shader_info, 0, &vertex_shader) != VK_SUCCESS) result = PIXELCHAR_ERROR_BACKEND_API;
		else vertex_shader_module_created = true;
	}

	shader_info.pCode = (uint32_t*)(customFragmentShaderSource == 0 ? fragment_shader_code : customFragmentShaderSource);
	shader_info.codeSize = (customFragmentShaderSource == 0 ? fragment_shader_code_len : customFragmentShaderSourceSize);

	if (result == PIXELCHAR_SUCCESS)
	{
		if (backend->func.vkCreateShaderModule(backend->device, &shader_info, 0, &fragment_shader) != VK_SUCCESS) result = PIXELCHAR_ERROR_BACKEND_API;
		else fragment_shader_module_created = true;
	}

	if (result == PIXELCHAR_SUCCESS)
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

		if (backend->func.vkCreateDescriptorSetLayout(backend->device, &set_layout_info, 0, &backend->set_layout) != VK_SUCCESS) result = PIXELCHAR_ERROR_BACKEND_API;
		else descriptor_set_layout_created = true;
	}

	if (result == PIXELCHAR_SUCCESS)
	{
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

		if (backend->func.vkCreatePipelineLayout(backend->device, &pipeline_layout_info, 0, &backend->pipe_layout) != VK_SUCCESS) result = PIXELCHAR_ERROR_BACKEND_API;
		else pipeline_layout_created = true;
	}

	if (result == PIXELCHAR_SUCCESS)
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
		pipe_info.layout = backend->pipe_layout;
		pipe_info.renderPass = backend->render_pass;
		pipe_info.subpass = backend->subpass;
		pipe_info.stageCount = 2;
		pipe_info.pRasterizationState = &rasterization_state;
		pipe_info.pViewportState = &viewport_state;
		pipe_info.pDynamicState = &dynamic_state;
		pipe_info.pMultisampleState = &multi_sample_state;
		pipe_info.pInputAssemblyState = &input_assembly;

		if (backend->func.vkCreateGraphicsPipelines(backend->device, 0, 1, &pipe_info, 0, &backend->pipeline) != VK_SUCCESS) result = PIXELCHAR_ERROR_BACKEND_API;
		else pipeline_created = true;
	}

	if (result == PIXELCHAR_SUCCESS)
	{
		VkCommandPoolCreateInfo pool_info = { 0 };
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.queueFamilyIndex = backend->queue_index;
		pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (backend->func.vkCreateCommandPool(backend->device, &pool_info, 0, &backend->cmd_pool) != VK_SUCCESS) result = PIXELCHAR_ERROR_BACKEND_API;
		else command_pool_created = true;

	}

	if (result == PIXELCHAR_SUCCESS)
	{
		VkCommandBufferAllocateInfo cmd_alloc_info = { 0 };
		cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd_alloc_info.commandBufferCount = 1;
		cmd_alloc_info.commandPool = backend->cmd_pool;
		cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		if (backend->func.vkAllocateCommandBuffers(backend->device, &cmd_alloc_info, &backend->cmd) != VK_SUCCESS) result = PIXELCHAR_ERROR_BACKEND_API;
		else command_buffers_allocated;
	}
	
	if (result == PIXELCHAR_SUCCESS)
	{
		VkDescriptorPoolSize pool_size = { 0 };
		pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		pool_size.descriptorCount = PIXELCHAR_RENDERER_MAX_FONT_COUNT;

		VkDescriptorPoolCreateInfo descriptor_pool_info = { 0 };
		descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptor_pool_info.maxSets = 1;
		descriptor_pool_info.poolSizeCount = 1;
		descriptor_pool_info.pPoolSizes = &pool_size;

		if (backend->func.vkCreateDescriptorPool(backend->device, &descriptor_pool_info, 0, &backend->descriptor_pool) != VK_SUCCESS) result = PIXELCHAR_ERROR_BACKEND_API;
		else descriptor_pool_created = true;
	}

	if (result == PIXELCHAR_SUCCESS)
	{
		VkDescriptorSetAllocateInfo descriptor_set_info = { 0 };
		descriptor_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptor_set_info.pSetLayouts = &backend->set_layout;
		descriptor_set_info.descriptorSetCount = 1;
		descriptor_set_info.descriptorPool = backend->descriptor_pool;

		if (backend->func.vkAllocateDescriptorSets(backend->device, &descriptor_set_info, &backend->descriptor_set) != VK_SUCCESS) result = PIXELCHAR_ERROR_BACKEND_API;
	}

	if (result == PIXELCHAR_SUCCESS)
	{
		VkFenceCreateInfo fence_info = { 0 };
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	
		if (backend->func.vkCreateFence(backend->device, &fence_info, 0, &backend->fence) != VK_SUCCESS) result = PIXELCHAR_ERROR_BACKEND_API;
		else fence_created = true;
	}

	if (result == PIXELCHAR_SUCCESS)
	{
		if (backend->func.vkMapMemory(backend->device, backend->staging_memory, 0, staging_buffer_size, 0, &backend->staging_buffer_host_handle) != VK_SUCCESS) result = PIXELCHAR_ERROR_BACKEND_API;
		else staging_memory_mapped = true;
	}

	if (result == PIXELCHAR_SUCCESS) if ((backend->vertex_index_memory_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) && (backend->vertex_index_memory_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
	{
		if (backend->func.vkMapMemory(backend->device, backend->vertex_index_memory, 0, vertex_index_buffer_size, 0, &backend->vertex_index_buffer_host_handle) != VK_SUCCESS) backend->vertex_index_memory_flags &= (~VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		else vertex_index_memory_mapped = true;
	}

	uint32_t index_data[] = {0, 1, 2, 1, 3, 2};

	if (result == PIXELCHAR_SUCCESS)
	{
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
		) result = PIXELCHAR_ERROR_BACKEND_API;
	}

	if (result != PIXELCHAR_SUCCESS && vertex_index_memory_mapped) backend->func.vkUnmapMemory(backend->device, backend->vertex_index_memory);
	if (result != PIXELCHAR_SUCCESS && staging_memory_mapped) backend->func.vkUnmapMemory(backend->device, backend->staging_memory);
	if (result != PIXELCHAR_SUCCESS && fence_created) backend->func.vkDestroyFence(backend->device, backend->fence, 0);
	if (result != PIXELCHAR_SUCCESS && descriptor_pool_created) backend->func.vkDestroyDescriptorPool(backend->device, backend->descriptor_pool, 0);
	if (result != PIXELCHAR_SUCCESS && command_buffers_allocated) backend->func.vkFreeCommandBuffers(backend->device, backend->cmd_pool, 1, &backend->cmd);
	if (result != PIXELCHAR_SUCCESS && command_pool_created) backend->func.vkDestroyCommandPool(backend->device, backend->cmd_pool, 0);
	if (result != PIXELCHAR_SUCCESS && pipeline_created) backend->func.vkDestroyPipeline(backend->device, backend->pipeline, 0);
	if (result != PIXELCHAR_SUCCESS && pipeline_layout_created) backend->func.vkDestroyPipelineLayout(backend->device, backend->pipe_layout, 0);
	if (result != PIXELCHAR_SUCCESS && descriptor_set_layout_created) backend->func.vkDestroyDescriptorSetLayout(backend->device, backend->set_layout, 0);
	if (fragment_shader_module_created) backend->func.vkDestroyShaderModule(backend->device, fragment_shader, 0);
	if (vertex_shader_module_created) backend->func.vkDestroyShaderModule(backend->device, vertex_shader, 0);
	if (result != PIXELCHAR_SUCCESS && staging_buffer_created) backend->func.vkDestroyBuffer(backend->device, backend->staging_buffer, 0);
	if (result != PIXELCHAR_SUCCESS && vertex_index_buffer_created) backend->func.vkDestroyBuffer(backend->device, backend->vertex_index_buffer, 0);
	if (result != PIXELCHAR_SUCCESS && staging_memory_allocated) backend->func.vkFreeMemory(backend->device, backend->staging_memory, 0);
	if (result != PIXELCHAR_SUCCESS && vertex_index_memory_allocated) backend->func.vkFreeMemory(backend->device, backend->vertex_index_memory, 0);
	if (result != PIXELCHAR_SUCCESS && backend_memory_allocated) free(backend);

	if (result == PIXELCHAR_SUCCESS)
	{
		renderer->backends[backendSlotIndex].data = backend;
		renderer->backends[backendSlotIndex].deinitialize_function = pixelcharRendererBackendVulkanDeinitialize;
		renderer->backends[backendSlotIndex].font_backend_add_reference_function = _font_backend_vulkan_add_reference;
		renderer->backends[backendSlotIndex].font_backend_sub_reference_function = _font_backend_vulkan_sub_reference;

		for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONT_COUNT; i++)
		{
			if (renderer->fonts[i] != NULL)
			{
				if (_font_backend_vulkan_add_reference(renderer, i, backendSlotIndex) == PIXELCHAR_SUCCESS)
					renderer->font_backends_referenced[i][backendSlotIndex] = true;
			}
		}
	}

	return result;
}

void pixelcharRendererBackendVulkanDeinitialize(PixelcharRenderer renderer, uint32_t backendSlotIndex)
{
	if (renderer == NULL) return;
	if (renderer->backends[backendSlotIndex].data == NULL) return;

	_renderer_backend_vulkan* backend = renderer->backends[backendSlotIndex].data;

	backend->func.vkDeviceWaitIdle(backend->device);

	for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONT_COUNT; i++)
	{
		if (renderer->fonts[i] != NULL && renderer->font_backends_referenced[i][backendSlotIndex] == true)
			_font_backend_vulkan_sub_reference(renderer, i, backendSlotIndex);
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
	renderer->backends[backendSlotIndex].data = NULL;
}

PixelcharResult pixelcharRendererBackendVulkanUpdateRenderingData(PixelcharRenderer renderer, uint32_t backendSlotIndex, uint32_t resourceFrameIndex, VkCommandBuffer commandBuffer)
{
	if (renderer == NULL) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;
	if (renderer->queue_filled_length == 0) return PIXELCHAR_SUCCESS;
	if (renderer->backends[backendSlotIndex].data == NULL) return PIXELCHAR_ERROR_BACKEND_SLOT_NOT_IN_USED;

	_renderer_backend_vulkan* backend = renderer->backends[backendSlotIndex].data;

	_pixelchar_renderer_convert_queue(renderer, backendSlotIndex);

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
			.size = VK_WHOLE_SIZE
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

	return PIXELCHAR_SUCCESS;
}

PixelcharResult pixelcharRendererBackendVulkanRender(
	PixelcharRenderer renderer,
	uint32_t backendSlotIndex,
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
	if (renderer->queue_filled_length == 0) return PIXELCHAR_SUCCESS;
	if (renderer->backends[backendSlotIndex].data == NULL) return PIXELCHAR_ERROR_BACKEND_SLOT_NOT_IN_USED;

	_renderer_backend_vulkan* backend = renderer->backends[backendSlotIndex].data;

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

	return PIXELCHAR_SUCCESS;
}