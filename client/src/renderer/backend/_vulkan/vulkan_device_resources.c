#include "vulkan_backend.h"

uint32_t floor_log2(uint32_t n) {
	uint32_t log = 0;
	while (n >>= 1) log++;
	return log;
}

#define max(x, y) (x > y ? x : y)

uint32_t vulkan_device_resources_rectangles_create(struct minec_client* game);
uint32_t vulkan_device_resources_rectangles_destroy(struct minec_client* game);

uint32_t vulkan_device_resources_create(struct minec_client* game) {


	pixelchar_renderer_backend_vulkan_init();

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
		image_info.format = VK_FORMAT_R8G8B8A8_UNORM;;
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

	size_t staging_buffer_size = max_image_memory_size;

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
		view_info.format = VK_FORMAT_R8G8B8A8_UNORM;
		view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view_info.subresourceRange.layerCount = 1;
		view_info.subresourceRange.levelCount = mip_level_counts[i];
		view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;

		VKCall(vkCreateImageView(game->renderer_state.backend.device, &view_info, 0, &game->renderer_state.backend.images[i].image_view));

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
		img_mem_barrier.subresourceRange = (VkImageSubresourceRange){ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

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
		copy_region.imageExtent = (VkExtent3D){ game->resource_state.image_atlas[i].width, game->resource_state.image_atlas[i].height, 1 };
		copy_region.imageSubresource.layerCount = 1;
		copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

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

	//samplers

	for (uint32_t i = 0; i < SAMPLERS_COUNT; i++) {

		VkSamplerCreateInfo sampler_info = { 0 };
		sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

		if (game->renderer_state.sampler_configurations[i].min_filter == SAMPLING_NEAREST) sampler_info.minFilter = VK_FILTER_NEAREST;
		if (game->renderer_state.sampler_configurations[i].min_filter == SAMPLING_LINEAR) sampler_info.minFilter = VK_FILTER_LINEAR;

		if (game->renderer_state.sampler_configurations[i].mag_filter == SAMPLING_NEAREST) sampler_info.magFilter = VK_FILTER_NEAREST;
		if (game->renderer_state.sampler_configurations[i].mag_filter == SAMPLING_LINEAR) sampler_info.magFilter = VK_FILTER_LINEAR;

		if (game->renderer_state.sampler_configurations[i].mipmap_mode == SAMPLING_NEAREST) sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		if (game->renderer_state.sampler_configurations[i].mipmap_mode == SAMPLING_LINEAR) sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		if (game->renderer_state.sampler_configurations[i].address_mode_u == SAMPLING_REPEAT) sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		if (game->renderer_state.sampler_configurations[i].address_mode_u == SAMPLING_CLAMP_TO_EDGE) sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

		if (game->renderer_state.sampler_configurations[i].address_mode_v == SAMPLING_REPEAT) sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		if (game->renderer_state.sampler_configurations[i].address_mode_v == SAMPLING_CLAMP_TO_EDGE) sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

		sampler_info.mipLodBias = game->renderer_state.sampler_configurations[i].mip_lod_bias;

		if (game->renderer_state.sampler_configurations[i].anisotropy_enable == SAMPLING_ENABLE) sampler_info.anisotropyEnable = VK_TRUE;
		if (game->renderer_state.sampler_configurations[i].anisotropy_enable == SAMPLING_DISABLE) sampler_info.anisotropyEnable = VK_FALSE;

		sampler_info.maxAnisotropy = game->renderer_state.sampler_configurations[i].max_anisotropy;

		if (game->renderer_state.sampler_configurations[i].compare_enable == SAMPLING_ENABLE) sampler_info.compareEnable = VK_TRUE;
		if (game->renderer_state.sampler_configurations[i].compare_enable == SAMPLING_DISABLE) sampler_info.compareEnable = VK_FALSE;

		if (game->renderer_state.sampler_configurations[i].compare_op == SAMPLING_COMPARE_NEVER) sampler_info.compareOp = VK_COMPARE_OP_NEVER;
		if (game->renderer_state.sampler_configurations[i].compare_op == SAMPLING_COMPARE_ALWAYS) sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
		if (game->renderer_state.sampler_configurations[i].compare_op == SAMPLING_COMPARE_EQUAL) sampler_info.compareOp = VK_COMPARE_OP_EQUAL;
		if (game->renderer_state.sampler_configurations[i].compare_op == SAMPLING_COMPARE_LESS_EQUAL) sampler_info.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		if (game->renderer_state.sampler_configurations[i].compare_op == SAMPLING_COMPARE_LESS) sampler_info.compareOp = VK_COMPARE_OP_LESS;
		if (game->renderer_state.sampler_configurations[i].compare_op == SAMPLING_COMPARE_GREATER_EQUAL) sampler_info.compareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
		if (game->renderer_state.sampler_configurations[i].compare_op == SAMPLING_COMPARE_GREATER) sampler_info.compareOp = VK_COMPARE_OP_GREATER;
		if (game->renderer_state.sampler_configurations[i].compare_op == SAMPLING_COMPARE_NOT_EQUAL) sampler_info.compareOp = VK_COMPARE_OP_NOT_EQUAL;

		sampler_info.minLod = game->renderer_state.sampler_configurations[i].min_lod;
		sampler_info.maxLod = game->renderer_state.sampler_configurations[i].max_lod;

		VKCall(vkCreateSampler(game->renderer_state.backend.device, &sampler_info, 0, &game->renderer_state.backend.samplers[i]));

	}


	//images/samplers descriptor set
	VkDescriptorSetLayoutBinding image_binding = { 0 };
	image_binding.binding = 0;
	image_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	image_binding.descriptorCount = RESOURCES_IMAGES_COUNT;
	image_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding sampler_binding = { 0 };
	sampler_binding.binding = 1;
	sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	sampler_binding.descriptorCount = SAMPLERS_COUNT;
	sampler_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding bindings[] = {
		image_binding,
		sampler_binding
	};

	VkDescriptorSetLayoutCreateInfo layout_create_info = { 0 };
	layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_create_info.bindingCount = 2;
	layout_create_info.pBindings = bindings;

	VKCall(vkCreateDescriptorSetLayout(game->renderer_state.backend.device, &layout_create_info, 0, &game->renderer_state.backend.images_descriptor_set_layout));

	VkDescriptorPoolSize pool_size_images = { 0 };
	pool_size_images.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	pool_size_images.descriptorCount = RESOURCES_IMAGES_COUNT;

	VkDescriptorPoolSize pool_size_samplers = { 0 };
	pool_size_samplers.type = VK_DESCRIPTOR_TYPE_SAMPLER;
	pool_size_samplers.descriptorCount = SAMPLERS_COUNT;

	VkDescriptorPoolSize pool_sizes[] = {
		pool_size_images,
		pool_size_samplers
	};

	VkDescriptorPoolCreateInfo pool_create_info = { 0 };
	pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_create_info.poolSizeCount = 2;
	pool_create_info.pPoolSizes = pool_sizes;
	pool_create_info.maxSets = 1;

	VKCall(vkCreateDescriptorPool(game->renderer_state.backend.device, &pool_create_info, 0, &game->renderer_state.backend.images_descriptor_pool));

	VkDescriptorSetAllocateInfo descriptor_set_alloc_info = { 0 };
	descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptor_set_alloc_info.descriptorPool = game->renderer_state.backend.images_descriptor_pool;
	descriptor_set_alloc_info.descriptorSetCount = 1;
	descriptor_set_alloc_info.pSetLayouts = &game->renderer_state.backend.images_descriptor_set_layout;

	VKCall(vkAllocateDescriptorSets(game->renderer_state.backend.device, &descriptor_set_alloc_info, &game->renderer_state.backend.images_descriptor_set));

	VkDescriptorImageInfo descriptor_image_infos[RESOURCES_IMAGES_COUNT];

	for (uint32_t i = 0; i < RESOURCES_IMAGES_COUNT; i++) {
		descriptor_image_infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		descriptor_image_infos[i].imageView = game->renderer_state.backend.images[i].image_view;
		descriptor_image_infos[i].sampler = VK_NULL_HANDLE;
	}

	VkDescriptorImageInfo descriptor_sampler_infos[SAMPLERS_COUNT];

	for (uint32_t i = 0; i < SAMPLERS_COUNT; i++) {
		descriptor_sampler_infos[i].imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		descriptor_sampler_infos[i].imageView = VK_NULL_HANDLE;
		descriptor_sampler_infos[i].sampler = game->renderer_state.backend.samplers[i];
	}

	VkWriteDescriptorSet descriptor_update_images_write = { 0 };
	descriptor_update_images_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_update_images_write.dstSet = game->renderer_state.backend.images_descriptor_set;
	descriptor_update_images_write.dstBinding = 0;
	descriptor_update_images_write.dstArrayElement = 0;
	descriptor_update_images_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	descriptor_update_images_write.descriptorCount = RESOURCES_IMAGES_COUNT;
	descriptor_update_images_write.pImageInfo = descriptor_image_infos;

	VkWriteDescriptorSet descriptor_update_samplers_write = { 0 };
	descriptor_update_samplers_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_update_samplers_write.dstSet = game->renderer_state.backend.images_descriptor_set;
	descriptor_update_samplers_write.dstBinding = 1;
	descriptor_update_samplers_write.dstArrayElement = 0;
	descriptor_update_samplers_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	descriptor_update_samplers_write.descriptorCount = SAMPLERS_COUNT;
	descriptor_update_samplers_write.pImageInfo = descriptor_sampler_infos;

	VkWriteDescriptorSet descriptor_update_writes[] = {
		descriptor_update_images_write,
		descriptor_update_samplers_write
	};

	vkUpdateDescriptorSets(game->renderer_state.backend.device, 2, descriptor_update_writes, 0, NULL);

	vkUnmapMemory(game->renderer_state.backend.device, staging_buffer_memory);

	vkFreeMemory(game->renderer_state.backend.device, staging_buffer_memory, 0);
	vkDestroyBuffer(game->renderer_state.backend.device, staging_buffer, 0);

	vulkan_device_resources_rectangles_create(game);

	return 0;
}

uint32_t vulkan_device_resources_destroy(struct minec_client* game) {

	vkDeviceWaitIdle(game->renderer_state.backend.device);

	vulkan_device_resources_rectangles_destroy(game);

	vkDestroyDescriptorPool(game->renderer_state.backend.device, game->renderer_state.backend.images_descriptor_pool, 0);
	vkDestroyDescriptorSetLayout(game->renderer_state.backend.device, game->renderer_state.backend.images_descriptor_set_layout, 0);

	for (uint32_t i = 0; i < SAMPLERS_COUNT; i++) {
		vkDestroySampler(game->renderer_state.backend.device, game->renderer_state.backend.samplers[i], 0);
	}

	for (uint32_t i = 0; i < RESOURCES_IMAGES_COUNT; i++) {

		vkDestroyImageView(game->renderer_state.backend.device, game->renderer_state.backend.images[i].image_view, 0);
		vkDestroyImage(game->renderer_state.backend.device, game->renderer_state.backend.images[i].image, 0);
	}

	vkFreeMemory(game->renderer_state.backend.device, game->renderer_state.backend.images_memory, 0);

	return 0;
}