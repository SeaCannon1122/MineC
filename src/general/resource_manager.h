#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "general/key_value.h"
#include "rendering/gui/pixel_char.h"

struct resource_manager_image {
	uint32_t width;
	uint32_t height;
	uint32_t* data;
};

struct resource_manager_shader {
	uint8_t* source_data;
	uint32_t source_data_size;
};

struct resource_manager {

	uint32_t image_count;
	struct resource_manager_image* images;
	void* images_name_map;

	uint32_t shader_count;
	struct resource_manager_shader* shaders;
	void* shaders_names_map;

	uint32_t pixelfont_count;
	struct pixel_font** pixelfonts;
	void* pixelfonts_names_map;

	uint32_t key_value_count;
	uint32_t audio_count;
};

uint32_t resource_manager_new(struct resource_manager* rm, uint8_t* file_path);

uint32_t resource_manager_destroy(struct resource_manager* rm);

uint32_t resource_manager_get_image(struct resource_manager* rm, uint8_t* name, struct resource_manager_image* image);
uint32_t resource_manager_get_shader(struct resource_manager* rm, uint8_t* name, struct resource_manager_shader* shader);
uint32_t resource_manager_get_pixelfont(struct resource_manager* rm, uint8_t* name, struct pixel_font** pixelfont);

#endif // !RESOURCE_MANAGER_H

