#ifndef RESOURCES_H
#define RESOURCES_H

#include <general/resource_manager.h>

enum resources_samplers {
	RESOURCE_SAMPLER_DEFAULT,
	RESOURCE_SAMPLER_INGAME_DEFAULT,
};

struct resources_sampler_configuration {
	VkFilter filer_min;
	VkFilter filer_mag;
	VkSamplerAddressMode address_mode_u;
	VkSamplerAddressMode address_mode_v;
	VkBool32 anisotropy_enable;
	float max_anisotropy;
	VkBool32 compare_enable;
	VkCompareOp compare_op;
};

static const struct resources_sampler_configuration resources_sampler_configurations[] = {
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
	RESOURCE_IMAGE_STONE
};

struct resources_image_configuration {
	uint8_t* resource_token;
	uint32_t sampler_id;
};

static const struct resources_image_configuration resources_image_configurations[] = {
	{"default", RESOURCE_SAMPLER_DEFAULT},
	{"dirt", RESOURCE_SAMPLER_DEFAULT},
	{"stone", RESOURCE_SAMPLER_DEFAULT},
};

#define RESOURCES_SAMPLER_COUNT sizeof(resources_sampler_configurations) / sizeof(resources_sampler_configurations[0])
#define RESOURCES_IMAGES_COUNT sizeof(resources_image_configurations) / sizeof(resources_image_configurations[0])

enum resources_shaders {
	RESOURCE_SHADER_MENU_VERTEX,
	RESOURCE_SHADER_MENU_FRAGMENT,
};

static uint8_t* resources_shader_tokens[] = {
	"menu_vertex",
	"menu_fragment"
};

#define RESOURCES_SHADERS_COUNT sizeof(resources_shader_tokens) / sizeof(resources_shader_tokens[0])


enum resources_pixelfonts {
	RESOURCE_PIXEL_FONT_DEFAULT,
	RESOURCE_PIXEL_FONT_DEBUG,
	RESOURCE_PIXEL_FONT_SPECIAL
};

static uint8_t* resources_pixelfont_tokens[] = {
	"default",
	"debug",
	"special"
};

#define RESOURCES_PIXELFONTS_COUNT sizeof(resources_pixelfont_tokens) / sizeof(resources_pixelfont_tokens[0])

struct resource_state {

	uint32_t device_used;

	uint8_t resource_manager_root_path[256];
	struct resource_manager resource_manager;

	//images
	uint32_t image_atlas[RESOURCES_IMAGES_COUNT];
	VkSampler samplers[RESOURCES_SAMPLER_COUNT];

	VkDescriptorImageInfo descriptor_image_infos[RESOURCES_IMAGES_COUNT];
	//shaders

	uint32_t shader_atlas[RESOURCES_SHADERS_COUNT];

	//pixelfonts
	uint32_t pixelfont_atlas[RESOURCES_PIXELFONTS_COUNT];

};

struct game_client;

uint32_t resources_create(struct game_client* game, uint32_t* resource_path);

uint32_t resources_switch_use_vulkan_device(struct game_client* game);

uint32_t resources_update_descriptor_set(struct game_client* game, VkDescriptorSet dst_set, uint32_t dst_binding, uint32_t dst_array_element);

uint32_t resources_switch_drop_vulkan_device(struct game_client* game);

uint32_t resources_destroy(struct game_client* game);

uint32_t resources_reload(struct game_client* game);

#endif // !RESOURCES_H
