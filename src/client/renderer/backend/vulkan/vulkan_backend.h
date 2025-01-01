#pragma once

#ifndef RENDERER_VULKAN_H
#define RENDERER_VULKAN_H

#include "general/platformlib/platform/platform.h"
#include "client/resources/resources.h"
#include "client/renderer/renderer_samplers.h"

#include "general/pixel_char/pixel_char.h"

#include <stdio.h>

#define VKCall(call) \
do { \
    VkResult result = (call); \
    if (result != VK_SUCCESS) { \
        printf("Vulkan error in \n    %s \n at %s:%d: %d\n", #call, __FILE__, __LINE__, result); \
		DEBUG_BREAK();\
    } \
} while(0)

struct renderer_backend {

	//instance
	VkPhysicalDevice gpus[16];
	uint32_t gpu_queue_indices[16];
	VkSurfaceFormatKHR gpu_surface_formats[16];

	VkInstance instance;
	VkDebugUtilsMessengerEXT debug_messenger;

	//surface
	VkSurfaceKHR surface;
	VkSurfaceFormatKHR surface_format;

	//device dependent

	//Device
	VkPhysicalDevice gpu;
	VkDevice device;
	VkQueue queue;
	uint32_t queue_index;

	uint32_t queue_used;

	VkCommandPool command_pool;
	VkCommandBuffer cmd;

	VkSemaphore submit_semaphore;
	VkSemaphore aquire_semaphore;
	VkFence queue_fence;

	VkRenderPass window_render_pass;

	//swapchain
	VkSwapchainKHR swapchain;
	uint32_t swapchain_image_count;
	VkImage swapchain_images[5];
	VkImageView swapchain_image_views[5];
	VkFramebuffer framebuffers[5];
	uint32_t swapchain_image_index;

	VkSurfaceCapabilitiesKHR surface_capabilities;
	VkPresentModeKHR present_mode;

	uint32_t resources_created;

	//resource dependent

	//images
	struct {
		VkImage image;
		VkImageView image_view;
		VkMemoryRequirements mem_requirements;
	} images[RESOURCES_IMAGES_COUNT];
	VkDeviceMemory images_memory;

	VkSampler samplers[SAMPLERS_COUNT];

	VkDescriptorSetLayout images_descriptor_set_layout;
	VkDescriptorPool images_descriptor_pool;
	VkDescriptorSet images_descriptor_set;

	uint32_t pixel_char_renderer_usable;
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
