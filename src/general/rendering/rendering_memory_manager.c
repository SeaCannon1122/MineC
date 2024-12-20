#include "rendering_memory_manager.h"

VkResult rendering_memory_manager_new(VkDevice device, VkPhysicalDevice gpu, VkQueue queue, uint32_t queue_index, size_t staging_buffer_size, struct rendering_memory_manager* rmm) {

	rmm->device = device;
	rmm->gpu = gpu;
	rmm->queue = queue;
	rmm->queue_index = queue_index;

	//command_pool

	VkCommandPoolCreateInfo pool_info = { 0 };
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = queue_index;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VKCall(vkCreateCommandPool(device, &pool_info, 0, &rmm->command_pool));

	//command buffer

	VkCommandBufferAllocateInfo cmd_alloc_info = { 0 };
	cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd_alloc_info.commandBufferCount = 1;
	cmd_alloc_info.commandPool = rmm->command_pool;
	cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	VKCall(vkAllocateCommandBuffers(device, &cmd_alloc_info, &rmm->cmd));

	VKCall(vkResetCommandBuffer(rmm->cmd, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));

	//fence 

	VkFenceCreateInfo fence_info = { 0 };
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	VKCall(vkCreateFence(rmm->device, &fence_info, 0, &rmm->cmd_fence));

	//staging buffer

	VkBufferCreateInfo buffer_info = { 0 };
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	buffer_info.size = staging_buffer_size;

	VKCall(vkCreateBuffer(device, &buffer_info, 0, &rmm->staging_buffer));

	VkMemoryRequirements memory_requirements;

	vkGetBufferMemoryRequirements(device, rmm->staging_buffer, &memory_requirements);

	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(gpu, &memory_properties);

	VkMemoryAllocateInfo alloc_info = { 0 };
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {

		if ((memory_requirements.memoryTypeBits & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) == (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
			alloc_info.memoryTypeIndex = i;
			break;
		}
	}

	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = staging_buffer_size;

	VKCall(vkAllocateMemory(device, &alloc_info, 0, &rmm->staging_buffer_memory));

	VKCall(vkBindBufferMemory(device, rmm->staging_buffer, rmm->staging_buffer_memory, 0));
	VKCall(vkMapMemory(device, rmm->staging_buffer_memory, 0, staging_buffer_size, 0, &rmm->staging_buffer_host_handle));

	return VK_SUCCESS;
}

VkResult rendering_memory_manager_destroy(struct rendering_memory_manager* rmm) {

	vkUnmapMemory(rmm->device, rmm->staging_buffer_memory);

	vkFreeMemory(rmm->device, rmm->staging_buffer_memory, 0);
	vkDestroyBuffer(rmm->device, rmm->staging_buffer, 0);

	vkDestroyFence(rmm->device, rmm->cmd_fence, 0);

	vkFreeCommandBuffers(rmm->device, rmm->command_pool, 1, &rmm->cmd);
	vkDestroyCommandPool(rmm->device, rmm->command_pool, 0);

	return VK_SUCCESS;
}

VkResult VkBuffer_new(struct rendering_memory_manager* rmm, uint32_t size, VkMemoryPropertyFlags property_flags, VkBufferUsageFlags usage_flags, struct rendering_buffer* buffer) {

	buffer->size = size;

	VkBufferCreateInfo buffer_info = { 0 };
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.usage = usage_flags;
	buffer_info.size = size;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VKCall(vkCreateBuffer(rmm->device, &buffer_info, 0, &buffer->buffer));

	VkMemoryRequirements memory_requirements;

	vkGetBufferMemoryRequirements(rmm->device, buffer->buffer, &memory_requirements);

	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(rmm->gpu, &memory_properties);

	VkMemoryAllocateInfo alloc_info = { 0 };
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {

		if ((memory_requirements.memoryTypeBits & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags) {
			alloc_info.memoryTypeIndex = i;
			buffer->property_flags = memory_properties.memoryTypes[i].propertyFlags;
			break;
		}
	}

	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = (size > memory_requirements.size ? size : memory_requirements.size);

	VKCall(vkAllocateMemory(rmm->device, &alloc_info, 0, &buffer->memory));
	VKCall(vkBindBufferMemory(rmm->device, buffer->buffer, buffer->memory, 0));

	if (buffer->property_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
		VKCall(vkMapMemory(rmm->device, buffer->memory, 0, size, 0, &buffer->memory_host_handle));
	}

	return VK_SUCCESS;
}

VkResult VkBuffer_fill(struct rendering_memory_manager* rmm, struct rendering_buffer* buffer, void* data, uint32_t size, uint32_t offset) {

	if (buffer->size - offset < size) return 1278384;

	if (buffer->property_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
		memcpy((size_t)buffer->memory_host_handle + (size_t)offset, data, size);

		if (buffer->property_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT == 0) {
			VkMappedMemoryRange memory_range = { 0 };
			memory_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			memory_range.memory = buffer->memory;
			memory_range.size = size;
			memory_range.offset = offset;

			VKCall(vkFlushMappedMemoryRanges(rmm->device, 1, &memory_range));
		}

	}
	else {
		memcpy(rmm->staging_buffer_host_handle, data, size);

		VKCall(vkResetFences(rmm->device, 1, &rmm->cmd_fence));

		VkCommandBufferBeginInfo begin_info = { 0 };
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VKCall(vkBeginCommandBuffer(rmm->cmd, &begin_info));

		VkBufferCopy copy_region = { 0 };
		copy_region.srcOffset = 0;
		copy_region.dstOffset = offset;
		copy_region.size = size;

		vkCmdCopyBuffer(rmm->cmd, rmm->staging_buffer, buffer->buffer, 1, &copy_region);

		VKCall(vkEndCommandBuffer(rmm->cmd));

		VkSubmitInfo submit_info = { 0 };
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &rmm->cmd;

		VKCall(vkQueueSubmit(rmm->queue, 1, &submit_info, rmm->cmd_fence));

		VKCall(vkWaitForFences(rmm->device, 1, &rmm->cmd_fence, 1, UINT64_MAX));

	}

	return VK_SUCCESS;
}

VkResult VkBuffer_destroy(struct rendering_memory_manager* rmm, struct rendering_buffer* buffer) {

	if (buffer->property_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
		vkUnmapMemory(rmm->device, buffer->memory);
	}

	vkDestroyBuffer(rmm->device, buffer->buffer, 0);
	vkFreeMemory(rmm->device, buffer->memory, 0);

	return VK_SUCCESS;
}

VkResult VkImage_new(struct rendering_memory_manager* rmm, uint32_t width, uint32_t height, VkFormat image_format, uint32_t pixel_size, VkImageUsageFlags usage, struct rendering_image* image) {

	image->width = width;
	image->height = height;
	image->pixel_size = pixel_size;

	VkImageCreateInfo image_info = { 0 };
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.format = image_format;
	image_info.extent = (VkExtent3D){ width, height, 1 };
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.usage = usage;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VKCall(vkCreateImage(rmm->device, &image_info, 0, &image->image));

	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(rmm->device, image->image, &memory_requirements);

	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(rmm->gpu, &memory_properties);

	VkMemoryAllocateInfo mem_alloc_info = { 0 };
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {

		if ((memory_requirements.memoryTypeBits & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
			mem_alloc_info.memoryTypeIndex = i;
			break;
		}
	}

	mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc_info.allocationSize = width * width * pixel_size;

	VKCall(vkAllocateMemory(rmm->device, &mem_alloc_info, 0, &image->memory));
	VKCall(vkBindImageMemory(rmm->device, image->image, image->memory, 0));


	VkImageViewCreateInfo view_info = { 0 };
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = image->image;
	view_info.format = image_format;
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	view_info.subresourceRange.layerCount = 1;
	view_info.subresourceRange.levelCount = 1;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;

	VKCall(vkCreateImageView(rmm->device, &view_info, 0, &image->view));

	return VK_SUCCESS;
}

VkResult VkImage_fill(struct rendering_memory_manager* rmm, void* image_data, VkImageUsageFlags usage, struct rendering_image* image) {

	memcpy(rmm->staging_buffer_host_handle, image_data, image->width * image->height * image->pixel_size);

	VKCall(vkResetFences(rmm->device, 1, &rmm->cmd_fence));

	VkCommandBufferBeginInfo begin_info = { 0 };
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VKCall(vkBeginCommandBuffer(rmm->cmd, &begin_info));

	VkImageSubresourceRange range = { 0 };
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	range.layerCount = 1;
	range.levelCount = 1;

	VkImageMemoryBarrier img_mem_barrier = { 0 };
	img_mem_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	img_mem_barrier.image = image->image;
	img_mem_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	img_mem_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	img_mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	img_mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	img_mem_barrier.subresourceRange = range;

	vkCmdPipelineBarrier(
		rmm->cmd,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0,
		0,
		0,
		0,
		1,
		&img_mem_barrier
	);

	VkBufferImageCopy copy_region = { 0 };
	copy_region.imageExtent = (VkExtent3D){ image->width, image->height, 1 };
	copy_region.imageSubresource.layerCount = 1;
	copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	vkCmdCopyBufferToImage(rmm->cmd, rmm->staging_buffer, image->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

	img_mem_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	img_mem_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	img_mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	img_mem_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(
		rmm->cmd,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0,
		0,
		0,
		0,
		1,
		&img_mem_barrier
	);

	VKCall(vkEndCommandBuffer(rmm->cmd));

	VkSubmitInfo submit_info = { 0 };
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &rmm->cmd;

	VKCall(vkQueueSubmit(rmm->queue, 1, &submit_info, rmm->cmd_fence));

	VKCall(vkWaitForFences(rmm->device, 1, &rmm->cmd_fence, 1, UINT64_MAX));

	return VK_SUCCESS;
}

VkResult VkImage_destroy(struct rendering_memory_manager* rmm, struct rendering_image* image) {
	vkDestroyImage(rmm->device, image->image, 0);
	vkFreeMemory(rmm->device, image->memory, 0);
	vkDestroyImageView(rmm->device, image->view, 0);

	return VK_SUCCESS;
}