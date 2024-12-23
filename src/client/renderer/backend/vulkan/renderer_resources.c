#include "../renderer_backend.h"

uint32_t floor_log2(uint32_t n) {
	uint32_t log = 0;
	while (n >>= 1) { // Right shift until n becomes 0
		log++;
	}
	return log;
}

uint32_t renderer_backend_load_resources(struct game_client* game) {

	size_t max_image_memory_size = 0;

	size_t images_memory_size = 0;
	uint32_t memory_bits = 0;

	uint32_t mip_level_counts[RESOURCES_IMAGES_COUNT];
		
	for (uint32_t i = 0; i < RESOURCES_IMAGES_COUNT; i++) {

		mip_level_counts[i] = floor_log2(max(game->resource_state.image_atlas[i].width, game->resource_state.image_atlas[i].height)) + 1;

		VkImageCreateInfo image_info = { 0 };
		image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.mipLevels = mip_level_counts[i];
		image_info.arrayLayers = 1;
		image_info.imageType = VK_IMAGE_TYPE_2D;
		image_info.format = VK_FORMAT_B8G8R8A8_SRGB;;
		image_info.extent = (VkExtent3D){ game->resource_state.image_atlas[i].width, game->resource_state.image_atlas[i].height, 1 };
		image_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VKCall(vkCreateImage(game->renderer_state.backend.device, &image_info, 0, &game->renderer_state.backend.images[i].image));

		vkGetImageMemoryRequirements(game->renderer_state.backend.device, game->renderer_state.backend.images[i].image, &game->renderer_state.backend.images[i].mem_requirements);

		images_memory_size += game->renderer_state.backend.images[i].mem_requirements.size;
		if (game->renderer_state.backend.images[i].mem_requirements.size > max_image_memory_size) max_image_memory_size = game->renderer_state.backend.images[i].mem_requirements.size;

		memory_bits = game->renderer_state.backend.images[i].mem_requirements.memoryTypeBits;
	}

	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(game->renderer_state.backend.gpu, &memory_properties);

	VkMemoryAllocateInfo mem_alloc_info = { 0 };
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {

		if ((memory_bits & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
			mem_alloc_info.memoryTypeIndex = i;
			break;
		}
	}

	mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc_info.allocationSize = images_memory_size;

	VKCall(vkAllocateMemory(game->renderer_state.backend.device, &mem_alloc_info, 0, &game->renderer_state.backend.images_memory));


	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	void* staging_buffer_host_handle;


	VKCall(vkResetCommandBuffer(game->renderer_state.backend.cmd, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));

	//staging buffer

	size_t staging_buffer_size = (sizeof(struct pixel_font) > max_image_memory_size ? sizeof(struct pixel_font) : max_image_memory_size);

	VkBufferCreateInfo buffer_info = { 0 };
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	buffer_info.size = staging_buffer_size;

	VKCall(vkCreateBuffer(game->renderer_state.backend.device, &buffer_info, 0, &staging_buffer));

	VkMemoryRequirements staging_memory_requirements;

	vkGetBufferMemoryRequirements(game->renderer_state.backend.device, staging_buffer, &staging_memory_requirements);

	VkMemoryAllocateInfo alloc_info = { 0 };
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {

		if ((staging_memory_requirements.memoryTypeBits & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) == (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
			alloc_info.memoryTypeIndex = i;
			break;
		}
	}

	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = staging_buffer_size;

	VKCall(vkAllocateMemory(game->renderer_state.backend.device, &alloc_info, 0, &staging_buffer_memory));

	VKCall(vkBindBufferMemory(game->renderer_state.backend.device, staging_buffer, staging_buffer_memory, 0));
	VKCall(vkMapMemory(game->renderer_state.backend.device, staging_buffer_memory, 0, staging_buffer_size, 0, &staging_buffer_host_handle));

	size_t memory_bound = 0;

	for (uint32_t i = 0; i < RESOURCES_IMAGES_COUNT; i++) {

		VKCall(vkBindImageMemory(game->renderer_state.backend.device, game->renderer_state.backend.images[i].image, game->renderer_state.backend.images_memory, memory_bound));

		VkImageViewCreateInfo view_info = { 0 };
		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image = game->renderer_state.backend.images[i].image;
		view_info.format = VK_FORMAT_B8G8R8A8_SRGB;
		view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view_info.subresourceRange.layerCount = 1;
		view_info.subresourceRange.levelCount = mip_level_counts[i];
		view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;

		VKCall(vkCreateImageView(game->renderer_state.backend.device, &view_info, 0, &game->renderer_state.backend.images[i].image_view));

		game->renderer_state.backend.images[i].mem_requirements = (VkMemoryRequirements){ 0 };

		memcpy(staging_buffer_host_handle, game->resource_state.image_atlas[i].data, game->resource_state.image_atlas[i].width * game->resource_state.image_atlas[i].height * 4);

		VKCall(vkResetFences(game->renderer_state.backend.device, 1, &game->renderer_state.backend.queue_fence));

		VkCommandBufferBeginInfo begin_info = { 0 };
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VKCall(vkBeginCommandBuffer(game->renderer_state.backend.cmd, &begin_info));


		VkImageMemoryBarrier img_mem_barrier = { 0 };
		img_mem_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		img_mem_barrier.image = game->renderer_state.backend.images[i].image;
		img_mem_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		img_mem_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		img_mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		img_mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		img_mem_barrier.subresourceRange = (VkImageSubresourceRange){ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

		vkCmdPipelineBarrier(
			game->renderer_state.backend.cmd,
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
		copy_region.imageExtent = (VkExtent3D){ game->resource_state.image_atlas[i].width, game->resource_state.image_atlas[i].height, 1};
		copy_region.imageSubresource.layerCount = 1;
		copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy_region.bufferOffset = memory_bound;

		vkCmdCopyBufferToImage(game->renderer_state.backend.cmd, staging_buffer, game->renderer_state.backend.images[i].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);


		img_mem_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		img_mem_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		img_mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		img_mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(
			game->renderer_state.backend.cmd,
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

		for (uint32_t j = 1; j < mip_level_counts[i]; j++) {

			VkImageBlit blit = { 0 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.layerCount = 1;
			blit.srcSubresource.mipLevel = j - 1;
			blit.srcOffsets[1].x = game->resource_state.image_atlas[i].width >> (j - 1);
			blit.srcOffsets[1].y = game->resource_state.image_atlas[i].height >> (j - 1);
			blit.srcOffsets[1].z = 1;

			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.layerCount = 1;
			blit.dstSubresource.mipLevel = j;
			blit.dstOffsets[1].x = game->resource_state.image_atlas[i].width >> j;
			blit.dstOffsets[1].y = game->resource_state.image_atlas[i].height >> j;
			blit.dstOffsets[1].z = 1;

			img_mem_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			img_mem_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			img_mem_barrier.srcAccessMask = 0;
			img_mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			img_mem_barrier.subresourceRange = (VkImageSubresourceRange){ VK_IMAGE_ASPECT_COLOR_BIT, j, 1, 0, 1 };

			vkCmdPipelineBarrier(
				game->renderer_state.backend.cmd,
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

			vkCmdBlitImage(
				game->renderer_state.backend.cmd,
				game->renderer_state.backend.images[i].image,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				game->renderer_state.backend.images[i].image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&blit,
				VK_FILTER_LINEAR
			);

			img_mem_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			img_mem_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			img_mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			img_mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			img_mem_barrier.subresourceRange = (VkImageSubresourceRange){ VK_IMAGE_ASPECT_COLOR_BIT, j, 1, 0, 1 };

			vkCmdPipelineBarrier(
				game->renderer_state.backend.cmd,
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

		}


		img_mem_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		img_mem_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		img_mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		img_mem_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		img_mem_barrier.subresourceRange = (VkImageSubresourceRange){ VK_IMAGE_ASPECT_COLOR_BIT, 0, mip_level_counts[i], 0, 1 };

		vkCmdPipelineBarrier(
			game->renderer_state.backend.cmd,
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

		VKCall(vkEndCommandBuffer(game->renderer_state.backend.cmd));

		VkSubmitInfo submit_info = { 0 };
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &game->renderer_state.backend.cmd;

		VKCall(vkQueueSubmit(game->renderer_state.backend.queue, 1, &submit_info, game->renderer_state.backend.queue_fence));

		VKCall(vkWaitForFences(game->renderer_state.backend.device, 1, &game->renderer_state.backend.queue_fence, 1, UINT64_MAX));

		memory_bound += game->renderer_state.backend.images[i].mem_requirements.size;
	}

	for (uint32_t i = 0; i < RENDERER_IMAGES_COUNT; i++) {
		game->renderer_state.backend.descriptor_image_infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		game->renderer_state.backend.descriptor_image_infos[i].imageView = game->renderer_state.backend.images[renderer_images[i].resource_image_index].image_view;
		game->renderer_state.backend.descriptor_image_infos[i].sampler = game->renderer_state.backend.samplers[renderer_images[i].sampling_configuarion];
	}


	//pixelfonts
	buffer_info.size = sizeof(struct pixel_font) * RESOURCES_PIXELFONTS_COUNT;
	buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	VKCall(vkCreateBuffer(game->renderer_state.backend.device, &buffer_info, 0, &game->renderer_state.backend.pixelfont_buffer));

	VkMemoryRequirements mem_requirements;
	vkGetBufferMemoryRequirements(game->renderer_state.backend.device, game->renderer_state.backend.pixelfont_buffer, &mem_requirements);

	alloc_info.allocationSize = mem_requirements.size;

	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {

		if ((mem_requirements.memoryTypeBits & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
			alloc_info.memoryTypeIndex = i;
			break;
		}
	}

	VKCall(vkAllocateMemory(game->renderer_state.backend.device, &alloc_info, 0, &game->renderer_state.backend.pixelfonts_memory));
	VKCall(vkBindBufferMemory(game->renderer_state.backend.device, game->renderer_state.backend.pixelfont_buffer, game->renderer_state.backend.pixelfonts_memory, 0));

	for (uint32_t i = 0; i < RESOURCES_PIXELFONTS_COUNT; i++) {

		memcpy(staging_buffer_host_handle, game->resource_state.pixelfont_atlas[i], sizeof(struct pixel_font));

		VKCall(vkResetFences(game->renderer_state.backend.device, 1, &game->renderer_state.backend.queue_fence));

		VkCommandBufferBeginInfo begin_info = { 0 };
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VKCall(vkBeginCommandBuffer(game->renderer_state.backend.cmd, &begin_info));

		VkBufferCopy copy_region = { 0 };
		copy_region.srcOffset = 0;
		copy_region.dstOffset = i * sizeof(struct pixel_font);
		copy_region.size = sizeof(struct pixel_font);

		vkCmdCopyBuffer(game->renderer_state.backend.cmd, staging_buffer, game->renderer_state.backend.pixelfont_buffer, 1, &copy_region);

		VKCall(vkEndCommandBuffer(game->renderer_state.backend.cmd));

		VkSubmitInfo submit_info = { 0 };
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &game->renderer_state.backend.cmd;

		VKCall(vkQueueSubmit(game->renderer_state.backend.queue, 1, &submit_info, game->renderer_state.backend.queue_fence));

		VKCall(vkWaitForFences(game->renderer_state.backend.device, 1, &game->renderer_state.backend.queue_fence, 1, UINT64_MAX));

		pixel_char_renderer_add_font(&game->renderer_state.backend.pcr, game->renderer_state.backend.pixelfont_buffer, i * sizeof(struct pixel_font), i);

	}

	vkUnmapMemory(game->renderer_state.backend.device, staging_buffer_memory);

	vkFreeMemory(game->renderer_state.backend.device, staging_buffer_memory, 0);
	vkDestroyBuffer(game->renderer_state.backend.device, staging_buffer, 0);

	


	return 0;
}

uint32_t update_descriptor_set_images(struct game_client* game, VkDescriptorSet dst_set, uint32_t dst_binding, uint32_t dst_array_element) {

	VkWriteDescriptorSet descriptor_set_update_write = { 0 };
	descriptor_set_update_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_set_update_write.dstSet = dst_set;
	descriptor_set_update_write.dstBinding = dst_binding;
	descriptor_set_update_write.dstArrayElement = dst_array_element;
	descriptor_set_update_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptor_set_update_write.descriptorCount = RESOURCES_IMAGES_COUNT;
	descriptor_set_update_write.pImageInfo = game->renderer_state.backend.descriptor_image_infos;

	vkUpdateDescriptorSets(game->renderer_state.backend.device, 1, &descriptor_set_update_write, 0, NULL);

	return 0;
}

uint32_t renderer_backend_unload_resources(struct game_client* game) {

	vkDestroyBuffer(game->renderer_state.backend.device, game->renderer_state.backend.pixelfont_buffer, 0);
	vkFreeMemory(game->renderer_state.backend.device, game->renderer_state.backend.pixelfonts_memory, 0);

	for (uint32_t i = 0; i < RESOURCES_IMAGES_COUNT; i++) {

		vkDestroyImageView(game->renderer_state.backend.device, game->renderer_state.backend.images[i].image_view, 0);
		vkDestroyImage(game->renderer_state.backend.device, game->renderer_state.backend.images[i].image, 0);
	}

	vkFreeMemory(game->renderer_state.backend.device, game->renderer_state.backend.images_memory, 0);

	return 0;
}