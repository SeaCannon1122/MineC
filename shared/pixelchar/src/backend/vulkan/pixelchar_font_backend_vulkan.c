#include "pixelchar_internal.h"

uint32_t _pixelchar_font_backend_vulkan_reference_add(struct pixelchar_font* font, struct pixelchar_renderer* pcr, uint32_t index)
{
	if (font->backends.vulkan.references == 0)
	{

		VkBufferCreateInfo buffer_info = { 0 };
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		buffer_info.size = font->bitmaps_count * font->resolution * font->resolution / 8;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CALL_FUNCTION(
			vkCreateBuffer(pcr->backends.vulkan.device, &buffer_info, 0, &font->backends.vulkan.buffer),
			_DEBUG_CALLBACK_CRITICAL_ERROR("_pixelchar_font_backend_vulkan_reference_add: vkCreateBuffer failed"),
			goto vkCreateBuffer_filed
		);

		VkMemoryRequirements memory_requirements;
		vkGetBufferMemoryRequirements(pcr->backends.vulkan.device, font->backends.vulkan.buffer, &memory_requirements);
		VkPhysicalDeviceMemoryProperties memory_properties;
		vkGetPhysicalDeviceMemoryProperties(pcr->backends.vulkan.gpu, &memory_properties);

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
			_DEBUG_CALLBACK_CRITICAL_ERROR("_pixelchar_font_backend_vulkan_reference_add: did not find suitable VkMemoryType for bitmap buffer");
			goto vkAllocateMemory_failed;
		}

		VK_CALL_FUNCTION(
			vkAllocateMemory(pcr->backends.vulkan.device, &alloc_info, 0, &font->backends.vulkan.memory),
			_DEBUG_CALLBACK_CRITICAL_ERROR("_pixelchar_font_backend_vulkan_reference_add: vkAllocateMemory failed"),
			goto vkAllocateMemory_failed
		);

		VK_CALL_FUNCTION(
			vkBindBufferMemory(pcr->backends.vulkan.device, font->backends.vulkan.buffer, font->backends.vulkan.memory, 0),
			_DEBUG_CALLBACK_CRITICAL_ERROR("_pixelchar_font_backend_vulkan_reference_add: vkBindBufferMemory failed"),
			goto vkBindBufferMemory_failed
		);

		VkDeviceSize offset = 0;
		VkDeviceSize total_size = font->bitmaps_count * font->resolution * font->resolution / 8;

		while (offset < total_size)
		{
			VkDeviceSize chunk_size = (PIXELCHAR_VULKAN_STAGING_SIZE < total_size - offset ? PIXELCHAR_VULKAN_STAGING_SIZE : total_size - offset);

			memcpy(pcr->backends.vulkan.staging_buffer_host_handle, (size_t)font->bitmaps + offset, chunk_size);

			VK_CALL_FUNCTION(
				vkResetCommandBuffer(pcr->backends.vulkan.cmd, 0),
				_DEBUG_CALLBACK_CRITICAL_ERROR("_pixelchar_font_backend_vulkan_reference_add: vkResetCommandBuffer failed"),
				goto vkBindBufferMemory_failed
			);

			VK_CALL_FUNCTION(
				vkResetFences(pcr->backends.vulkan.device, 1, &pcr->backends.vulkan.fence),
				_DEBUG_CALLBACK_CRITICAL_ERROR("_pixelchar_font_backend_vulkan_reference_add: vkResetFences failed"),
				goto vkBindBufferMemory_failed
			);

			VkCommandBufferBeginInfo begin_info = { 0 };
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			VK_CALL_FUNCTION(
				vkBeginCommandBuffer(pcr->backends.vulkan.cmd, &begin_info),
				_DEBUG_CALLBACK_CRITICAL_ERROR("_pixelchar_font_backend_vulkan_reference_add: vkBeginCommandBuffer failed"),
				goto vkBindBufferMemory_failed
			);

			VkBufferCopy copy_region = { 0 };
			copy_region.srcOffset = 0;
			copy_region.dstOffset = offset;
			copy_region.size = chunk_size;

			vkCmdCopyBuffer(pcr->backends.vulkan.cmd, pcr->backends.vulkan.staging_buffer, font->backends.vulkan.buffer, 1, &copy_region);


			VK_CALL_FUNCTION(
				vkEndCommandBuffer(pcr->backends.vulkan.cmd),
				_DEBUG_CALLBACK_CRITICAL_ERROR("_pixelchar_font_backend_vulkan_reference_add: vkEndCommandBuffer failed"),
				goto vkBindBufferMemory_failed
			);

			// Submit
			VkSubmitInfo submit_info = { 0 };
			submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.commandBufferCount = 1;
			submit_info.pCommandBuffers = &pcr->backends.vulkan.cmd;

			VK_CALL_FUNCTION(
				vkQueueSubmit(pcr->backends.vulkan.queue, 1, &submit_info, pcr->backends.vulkan.fence),
				_DEBUG_CALLBACK_CRITICAL_ERROR("_pixelchar_font_backend_vulkan_reference_add: vkQueueSubmit failed"),
				goto vkBindBufferMemory_failed
			);

			vkWaitForFences(pcr->backends.vulkan.device, 1, &pcr->backends.vulkan.fence, 1, UINT64_MAX);

			offset += chunk_size;
		}

		font->backends.vulkan.device = pcr->backends.vulkan.device;
	}

	VkDescriptorBufferInfo buffer_info = { 0 };
	buffer_info.buffer = font->backends.vulkan.buffer;
	buffer_info.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet buffer_write = { 0 };
	buffer_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	buffer_write.dstSet = pcr->backends.vulkan.descriptor_set;
	buffer_write.pBufferInfo = &buffer_info;
	buffer_write.dstBinding = index;
	buffer_write.descriptorCount = 1;
	buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

	vkUpdateDescriptorSets(pcr->backends.vulkan.device, 1, &buffer_write, 0, 0);

	font->backends.vulkan.references++;

	return PIXELCHAR_SUCCESS;

vkBindBufferMemory_failed:
	vkFreeMemory(pcr->backends.vulkan.device, font->backends.vulkan.memory, 0);
vkAllocateMemory_failed:
	vkDestroyBuffer(pcr->backends.vulkan.device, font->backends.vulkan.buffer, 0);
vkCreateBuffer_filed:
	return PIXELCHAR_FAILED;
}

void _pixelchar_font_backend_vulkan_reference_subtract(struct pixelchar_font* font)
{
	if (font->backends.vulkan.references == 1)
	{
		vkFreeMemory(font->backends.vulkan.device, font->backends.vulkan.memory, 0);
		vkDestroyBuffer(font->backends.vulkan.device, font->backends.vulkan.buffer, 0);
	}
	
	font->backends.vulkan.references--;
}

void _pixelchar_font_backend_vulkan_reference_init(struct pixelchar_font* font)
{
	font->backends.vulkan.references = 0;
}