#pragma once

#ifndef RENDERER_VULKAN_H
#define RENDERER_VULKAN_H

#include <stdint.h>
#include "general/platformlib/platform/platform.h"
#include "general/rendering/rendering_memory_manager.h"
#include "client/resources/resources.h"

#include "general/rendering/gui/pixel_char.h"
	
struct renderer_backend {

	VkPhysicalDevice gpus[16];
	uint32_t gpu_queue_indices[16];
	VkSurfaceFormatKHR gpu_surface_formats[16];


	int32_t dummy_window;
	VkSurfaceFormatKHR surface_format;

	VkInstance instance;
	VkDebugUtilsMessengerEXT debug_messenger;

	//Device
	VkPhysicalDevice gpu;
	VkDevice device;
	VkQueue queue;
	uint32_t queue_index;

	struct rendering_memory_manager rmm;

	VkCommandPool command_pool;
	VkCommandBuffer cmd;

	VkSemaphore submit_semaphore;
	VkSemaphore aquire_semaphore;
	VkFence queue_fence;

	VkRenderPass window_render_pass;

	VkSampler samplers[RESOURCES_SAMPLERS_COUNT];

	//images
	struct {
		VkImage image;
		VkImageView image_view;
		VkMemoryRequirements mem_requirements;
	} images[RESOURCES_IMAGES_COUNT];
	VkDeviceMemory images_memory;

	VkDescriptorSetLayout images_descriptor_set_layout;
	VkDescriptorPool images_descriptor_pool;
	VkDescriptorSet images_descriptor_set;

	struct pixel_char_renderer pcr;
	VkDeviceMemory pixelfonts_memory;
	VkBuffer pixelfont_buffer;

	uint32_t rectangles_pipeline_usable_bool;
	VkPipelineLayout rectangles_pipeline_layout;
	VkPipeline rectangles_pipeline;

	VkBuffer rectangles_buffer;
	VkDeviceMemory rectangles_buffer_memory;
	void* rectangles_buffer_memory_host_handle;

	uint32_t rectangles_count;

};

#endif // !RENDERER_VULKAN_H
