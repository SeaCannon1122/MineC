#ifndef RESOURCES_H
#define RESOURCES_H

#include <general/resource_manager.h>

enum resources_samplers {
	RESOURCES_SAMPLER_DEFAULT,
	RESOURCES_SAMPLER_INGAME_DEFAULT,
};

struct resources_sampler_configuration {
	VkFilter filer_min;
	VkFilter filer_mag;
	VkSamplerAddressMode address_mode_x;
	VkSamplerAddressMode address_mode_y;
	VkBool32 anisotropyEnable;
	float maxAnisotropy;
	VkBool32 compareEnable;
	VkCompareOp compareOp;
};

static struct resources_sampler_configuration resources_sampler_configurations[] = {
	{
		VK_FILTER_NEAREST, 
		VK_FILTER_NEAREST,
		VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VK_SAMPLER_ADDRESS_MODE_REPEAT,
		0,
		0.f,
		0,
		0
	}
};

enum resources_images {
	RESOURCE_IMAGE_DEAFULT,
	RESOURCE_IMAGE_DIRT,
};

struct resources_image_configuration {
	uint8_t* resource_token;
	uint32_t sampler_id;
};

static struct resources_image_configuration resources_image_configurations[] = {
	{"default", RESOURCES_SAMPLER_DEFAULT},
	{"dirt", RESOURCES_SAMPLER_DEFAULT},
};

#define SAMPLER_COUNT sizeof(resources_sampler_configurations) / sizeof(resources_sampler_configurations[0])
#define TEXTURE_ATLAS_LENGTH sizeof(resources_image_configurations) / sizeof(resources_image_configurations[0])

struct resource_state {

	uint8_t resource_manager_root_path[256];
	struct resource_manager resource_manager;

	uint32_t texture_atlas[TEXTURE_ATLAS_LENGTH];
	VkSampler samplers[SAMPLER_COUNT];

};

uint32_t resources_create(struct game_client* game, uint32_t* resource_path);

uint32_t resources_switch_use_vulkan_device(struct game_client* game);

uint32_t resources_switch_drop_vulkan_device(struct game_client* game);

uint32_t resources_destroy(struct game_client* game);

uint32_t resources_reload(struct game_client* game);

#endif // !RESOURCES_H
