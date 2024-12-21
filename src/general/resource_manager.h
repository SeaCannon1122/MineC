#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "general/key_value.h"
#include <vulkan/vulkan.h>
#include "rendering/gui/pixel_char.h"

struct resource_manager_image {
	
	uint32_t width;
	uint32_t height;
	uint32_t* data;

	VkImage image;
	VkImageView image_view;
	VkMemoryRequirements memory_requirements;
};

struct resource_manager_shader {
	uint8_t* source_data;
	uint32_t source_data_size;
	VkShaderModule shader_module;
};

struct resource_manager {

	uint32_t image_count;
	struct resource_manager_image* images;
	VkDeviceMemory images_memory;
	void* images_name_map;

	uint32_t shader_count;
	struct resource_manager_shader* shaders;
	void* shaders_names_map;

	uint32_t pixelfont_count;
	struct pixel_font** pixelfonts;
	void* pixelfonts_names_map;

	uint32_t key_value_count;
	uint32_t audio_count;

	VkDevice device;
	VkPhysicalDevice gpu;
	VkQueue queue;
	uint32_t queue_index;
};

uint32_t resource_manager_new(struct resource_manager* rm, uint8_t* file_path);

uint32_t resource_manager_use_vulkan_device(struct resource_manager* rm, VkDevice device, VkPhysicalDevice gpu, VkQueue queue, uint32_t queue_index);
uint32_t resource_manager_drop_vulkan_device(struct resource_manager* rm);

uint32_t resource_manager_destroy(struct resource_manager* rm);

int32_t resource_manager_get_image_index(struct resource_manager* rm, uint8_t* name);
int32_t resource_manager_get_shader_index(struct resource_manager* rm, uint8_t* name);
int32_t resource_manager_get_pixelfont_index(struct resource_manager* rm, uint8_t* name);

#endif // !RESOURCE_MANAGER_H

