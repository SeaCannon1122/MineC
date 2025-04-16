#include "vulkan/vulkan_backend.h"

uint32_t renderer_backend_use_gpu(struct minec_client* game, uint32_t gpu_index)
{
	vulkan_backend_use_gpu(game, gpu_index);
}

uint32_t renderer_backend_create(struct minec_client* game)
{
	vulkan_backend_create(game);
}

uint32_t renderer_backend_destroy(struct minec_client* game)
{
	vulkan_backend_destroy(game);
}

uint32_t renderer_backend_resize(struct minec_client* game)
{
	vulkan_backend_resize(game);
}

uint32_t renderer_backend_load_resources(struct minec_client* game)
{
	vulkan_backend_load_resources(game);
}

uint32_t renderer_backend_unload_resources(struct minec_client* game)
{
	vulkan_backend_unload_resources(game);
}

uint32_t renderer_backend_add_pixel_chars(struct minec_client* game, struct pixel_char* chars, uint32_t chars_count)
{
	vulkan_backend_add_pixel_chars(game, chars, chars_count);
}

uint32_t renderer_backend_add_rectangles(struct minec_client* game, struct renderer_rectangle* rectangles, uint32_t rectangles_count)
{
	vulkan_backend_add_rectangles(game, rectangles, rectangles_count);
}

uint32_t renderer_backend_render(struct minec_client* game)
{
	vulkan_backend_render(game);
}