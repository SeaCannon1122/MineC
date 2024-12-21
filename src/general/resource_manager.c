#include "resource_manager.h"

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <STB_IMAGE/stb_image.h>

#include "general/rendering/vulkan_helpers.h"

void* layout_maps[32];
uint8_t layout_maps_paths[32][512];
uint32_t layout_maps_length;

uint32_t error;

uint32_t image_file_count;
uint32_t key_value_file_count;
uint32_t audio_file_count;
uint32_t shader_file_count;
uint32_t pixelfont_file_count;

uint32_t default_image[] = {
	0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff,   0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
	0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff,   0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
	0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff,   0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
	0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff,   0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
	0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff,   0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
	0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff,   0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
	0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff,   0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
	0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff,   0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,

	0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,   0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff,
	0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,   0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff,
	0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,   0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff,
	0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,   0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff,
	0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,   0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff,
	0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,   0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff,
	0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,   0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff,
	0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,   0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff, 0xeff800ff,
};
uint32_t default_image_width = 16;
uint32_t default_image_height = 16;


uint32_t _load_resource_layout(uint8_t* file_path) {

	uint8_t sub_path[512];

	uint32_t layout_maps_index = layout_maps_length;

	sprintf(&layout_maps_paths[layout_maps_index][0], file_path);

	layout_maps[layout_maps_index] = key_value_new(100, 1000);
	layout_maps_length++;
	enum key_value_return_type load_return_type = key_value_load_yaml(&layout_maps[layout_maps_index], file_path);

	if (load_return_type & KEY_VALUE_ERROR_MASK) { error = load_return_type;  return layout_maps_index; }

	sprintf(sub_path, file_path);

	int32_t sub_path_length = strlen(file_path);

	for (; sub_path_length >= 0; sub_path_length--) {
		if (file_path[sub_path_length] == '/') break;
		sub_path[sub_path_length] = 0;
	}

	sub_path_length++;

	uint32_t map_size = key_value_get_size(layout_maps[layout_maps_index]);

	for (uint32_t i = 0; i < map_size; i++) {

		struct key_value_pair pair;
		key_value_get_pair(layout_maps[layout_maps_index], &pair, i);

		int32_t value = strlen(pair.value.string);

		uint32_t dot1 = -1;
		for (; value >= 0; value--) {
			if (pair.value.string[value] == '/') break;
			if (pair.value.string[value] == '.') {
				dot1 = value;
				value--;
				break;
			}
		}
		if (dot1 == -1) continue;

		

		int32_t dot2 = -1;
		for (; value >= 0; value--) {
			if (pair.value.string[value] == '/') break;
			if (pair.value.string[value] == '.') {
				dot2 = value;
				value++;
				break;
			}
		}

		if (dot1 - dot2 != sizeof("resourcelayout")) {

			if (strcmp(&pair.value.string[dot1 + 1], "png") == 0) image_file_count++;
			else if (strcmp(&pair.value.string[dot1 + 1], "yaml") == 0) key_value_file_count++;
			else if (strcmp(&pair.value.string[dot1 + 1], "mp3") == 0) audio_file_count++;
			else if (strcmp(&pair.value.string[dot1 + 1], "spv") == 0) shader_file_count++;
			else if (strcmp(&pair.value.string[dot1 + 1], "pixelfont") == 0) pixelfont_file_count++;

			continue;
		}

		int32_t resource_layout = 1;

		for (; value < dot1; value++) {
			if ("resourcelayout"[value - dot2 - 1] != pair.value.string[value]) {
				resource_layout = 0;
				break;
			}
		}

		if (resource_layout == 0) continue;

		sprintf(&sub_path[sub_path_length], pair.value.string);

		uint32_t ret_val = _load_resource_layout(sub_path);

		if (error) return ret_val;

	}


}

uint32_t resource_manager_new(struct resource_manager* rm, uint8_t* file_path) {

	rm->device = 0;

	layout_maps_length = 0;
	error = 0;

	image_file_count = 0;
	key_value_file_count = 0;
	audio_file_count = 0;
	shader_file_count = 0;
	pixelfont_file_count = 0;

	uint32_t error_index = _load_resource_layout(file_path);


	if (error) {
		for (uint32_t i = 0; i < layout_maps_length; i++) free(layout_maps[i]);

		if (error == KEY_VALUE_ERROR_COULDNT_OPEN_FILE) printf("[RESOURCE MANAGER] Couldn't open resource layout file ");
		else if (error == KEY_VALUE_ERROR_FILE_INVALID_SYNTAX) printf("[RESOURCE MANAGER] Invalid syntax in resource layout file ");

		printf("%s\n", &layout_maps_paths[error_index][0]);

		return NULL; 
	}

	rm->image_count = 1;
	rm->images = malloc(sizeof(struct resource_manager_image) * (image_file_count + 1));
	rm->images_name_map = key_value_new(image_file_count + 1, image_file_count * 20);
	key_value_set_integer(&rm->images_name_map, "default", 0);
	rm->images[0].data = default_image;
	rm->images[0].width = default_image_width;
	rm->images[0].height = default_image_height;

	rm->key_value_count = 0;
	rm->audio_count = 0;
	rm->shader_count = 0;
	rm->pixelfont_count = 0;

	uint8_t sub_path[512];

	for (uint32_t i = 0; i < layout_maps_length; i++) {
		uint32_t size = key_value_get_size(layout_maps[i]);

		sprintf(sub_path, &layout_maps_paths[i][0]);

		int32_t sub_path_length = strlen(&layout_maps_paths[i][0]);

		for (; sub_path_length >= 0; sub_path_length--) {
			if (layout_maps_paths[i][sub_path_length] == '/') break;
			sub_path[sub_path_length] = 0;
		}

		sub_path_length++;

		for (uint32_t key_i = 0; key_i < size; key_i++) {

			struct key_value_pair pair;
			key_value_get_pair(layout_maps[i], &pair, key_i);

			int32_t value = strlen(pair.value.string);
			sprintf(&sub_path[sub_path_length], pair.value.string);

			uint32_t dot1 = -1;
			for (; value >= 0; value--) {
				if (pair.value.string[value] == '/') break;
				if (pair.value.string[value] == '.') {
					dot1 = value;
					value--;
					break;
				}
			}
			if (dot1 == -1) continue;
			

			if (strcmp(&pair.value.string[dot1 + 1], "png") == 0) {
				uint32_t comp;

				rm->images[rm->image_count].data = stbi_load(sub_path, &rm->images[rm->image_count].width, &rm->images[rm->image_count].height, &comp, 0);

				if (rm->images[rm->image_count].data == NULL) {

					printf("[RESOURCE MANAGER] Couldn't load file %s from %s\n", sub_path, &layout_maps_paths[i][0]);

				}

				else {
					key_value_set_integer(&rm->images_name_map, pair.key, rm->image_count);

					rm->image_count++;
				}
			}


			else if (strcmp(&pair.value.string[dot1 + 1], "yaml") == 0) {

			}
			else if (strcmp(&pair.value.string[dot1 + 1], "mp3") == 0) {

			}
			else if (strcmp(&pair.value.string[dot1 + 1], "spv") == 0) {

			}
			else if (strcmp(&pair.value.string[dot1 + 1], "pixelfont") == 0) {

			}

		}

	}

	return 0;
}

uint32_t resource_manager_use_vulkan_device(struct resource_manager* rm, VkDevice device, VkPhysicalDevice gpu, VkQueue queue, uint32_t queue_index) {

	rm->device = device;
	rm->gpu = gpu;
	rm->queue = queue;
	rm->queue_index = queue_index;

	size_t max_image_memory_size = 0;

	size_t images_memory_size = 0;
	uint32_t memory_bits = 0;

	for (uint32_t i = 0; i < rm->image_count; i++) {

		VkImageCreateInfo image_info = { 0 };
		image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.mipLevels = 1;
		image_info.arrayLayers = 1;
		image_info.imageType = VK_IMAGE_TYPE_2D;
		image_info.format = VK_FORMAT_B8G8R8A8_SRGB;;
		image_info.extent = (VkExtent3D){ rm->images[i].width, rm->images[i].height, 1 };
		image_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VKCall(vkCreateImage(device, &image_info, 0, &rm->images[i].image));

		vkGetImageMemoryRequirements(device, rm->images[i].image, &rm->images[i].memory_requirements);

		images_memory_size += rm->images[i].memory_requirements.size;
		if (rm->images[i].memory_requirements.size > max_image_memory_size) max_image_memory_size = rm->images[i].memory_requirements.size;

		memory_bits = rm->images[i].memory_requirements.memoryTypeBits;
	}



	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(rm->gpu, &memory_properties);

	VkMemoryAllocateInfo mem_alloc_info = { 0 };
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {

		if ((memory_bits & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
			mem_alloc_info.memoryTypeIndex = i;
			break;
		}
	}

	mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc_info.allocationSize = images_memory_size;

	VKCall(vkAllocateMemory(device, &mem_alloc_info, 0, &rm->images_memory));

	VkCommandPool command_pool;

	VkCommandBuffer cmd;
	VkFence cmd_fence;

	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	void* staging_buffer_host_handle;


	//command_pool

	VkCommandPoolCreateInfo pool_info = { 0 };
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = queue_index;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VKCall(vkCreateCommandPool(device, &pool_info, 0, &command_pool));

	//command buffer

	VkCommandBufferAllocateInfo cmd_alloc_info = { 0 };
	cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd_alloc_info.commandBufferCount = 1;
	cmd_alloc_info.commandPool = command_pool;
	cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	VKCall(vkAllocateCommandBuffers(device, &cmd_alloc_info, &cmd));

	VKCall(vkResetCommandBuffer(cmd, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));

	//fence 

	VkFenceCreateInfo fence_info = { 0 };
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	VKCall(vkCreateFence(device, &fence_info, 0, &cmd_fence));

	//staging buffer

	size_t staging_buffer_size = (sizeof(struct pixel_font) > max_image_memory_size ? sizeof(struct pixel_font) : max_image_memory_size);

	VkBufferCreateInfo buffer_info = { 0 };
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	buffer_info.size = staging_buffer_size;

	VKCall(vkCreateBuffer(device, &buffer_info, 0, &staging_buffer));

	VkMemoryRequirements memory_requirements;

	vkGetBufferMemoryRequirements(device, staging_buffer, &memory_requirements);

	VkPhysicalDeviceMemoryProperties staging_memory_properties;
	vkGetPhysicalDeviceMemoryProperties(gpu, &staging_memory_properties);

	VkMemoryAllocateInfo alloc_info = { 0 };
	for (uint32_t i = 0; i < staging_memory_properties.memoryTypeCount; i++) {

		if ((memory_requirements.memoryTypeBits & (1 << i)) && (staging_memory_properties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) == (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
			alloc_info.memoryTypeIndex = i;
			break;
		}
	}

	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = staging_buffer_size;

	VKCall(vkAllocateMemory(device, &alloc_info, 0, &staging_buffer_memory));

	VKCall(vkBindBufferMemory(device, staging_buffer, staging_buffer_memory, 0));
	VKCall(vkMapMemory(device, staging_buffer_memory, 0, staging_buffer_size, 0, &staging_buffer_host_handle));

	size_t memory_bound = 0;

	for (uint32_t i = 0; i < rm->image_count; i++) {

		VKCall(vkBindImageMemory(device, rm->images[i].image, rm->images_memory, memory_bound));

		VkImageViewCreateInfo view_info = { 0 };
		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image = rm->images[i].image;
		view_info.format = VK_FORMAT_B8G8R8A8_SRGB;
		view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view_info.subresourceRange.layerCount = 1;
		view_info.subresourceRange.levelCount = 1;
		view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;

		VKCall(vkCreateImageView(device, &view_info, 0, &rm->images[i].image_view));

		rm->images[i].memory_requirements = (VkMemoryRequirements){ 0 };

		memcpy(staging_buffer_host_handle, rm->images[i].data, rm->images[i].width * rm->images[i].height * 4);

		VKCall(vkResetFences(device, 1, &cmd_fence));

		VkCommandBufferBeginInfo begin_info = { 0 };
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VKCall(vkBeginCommandBuffer(cmd, &begin_info));

		VkImageSubresourceRange range = { 0 };
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.layerCount = 1;
		range.levelCount = 1;

		VkImageMemoryBarrier img_mem_barrier = { 0 };
		img_mem_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		img_mem_barrier.image = rm->images[i].image;
		img_mem_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		img_mem_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		img_mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		img_mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		img_mem_barrier.subresourceRange = range;

		vkCmdPipelineBarrier(
			cmd,
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
		copy_region.imageExtent = (VkExtent3D){ rm->images[i].width, rm->images[i].height, 1 };
		copy_region.imageSubresource.layerCount = 1;
		copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy_region.bufferOffset = memory_bound;

		vkCmdCopyBufferToImage(cmd, staging_buffer, rm->images[i].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

		img_mem_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		img_mem_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		img_mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		img_mem_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(
			cmd,
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

		VKCall(vkEndCommandBuffer(cmd));

		VkSubmitInfo submit_info = { 0 };
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &cmd;

		VKCall(vkQueueSubmit(queue, 1, &submit_info, cmd_fence));

		VKCall(vkWaitForFences(device, 1, &cmd_fence, 1, UINT64_MAX));

		memory_bound += rm->images[i].memory_requirements.size;
	}

	
	vkUnmapMemory(device, staging_buffer_memory);

	vkFreeMemory(device, staging_buffer_memory, 0);
	vkDestroyBuffer(device, staging_buffer, 0);

	vkDestroyFence(device, cmd_fence, 0);

	vkFreeCommandBuffers(device, command_pool, 1, &cmd);
	vkDestroyCommandPool(device, command_pool, 0);

	return 0;
}

uint32_t resource_manager_drop_vulkan_device(struct resource_manager* rm) {

	if (rm->image_count) {

		for (uint32_t i = 0; i < rm->image_count; i++) {

			vkDestroyImage(rm->device, rm->images[i].image, 0);
			vkDestroyImageView(rm->device, rm->images[i].image_view, 0);
		}

		vkFreeMemory(rm->device, rm->images_memory, 0);
	}

	rm->device = 0;

	return 0;
}

uint32_t resource_manager_destroy(struct resource_manager* rm) {

	if (rm->device) resource_manager_drop_vulkan_device(rm);

	return 0;
}


uint32_t resource_manager_get_image_index(struct resource_manager* rm, uint8_t* name) {

	uint64_t image_index;
	key_value_get_integer(&rm->images_name_map, name, 0, &image_index);

	return image_index;
}