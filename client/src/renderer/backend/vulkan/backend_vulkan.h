#pragma once

#ifndef MINEC_CLIENT_RENDERER_BACKEND_VULKAN_BACKEND_VULKAN_H
#define MINEC_CLIENT_RENDERER_BACKEND_VULKAN_BACKEND_VULKAN_H

#include <cwindow/cwindow.h>

#define VULKAN_FUNC RENDERER.components.backend.vulkan.func
#define VULKAN RENDERER.components.backend.vulkan

#define VULKAN_RESOURCE_FRAME_COUNT 3

#define VULKAN_PRE_INSTANCE_FUNCTION_LIST \
	VULKAN_FUNCTION(vkEnumerateInstanceExtensionProperties)\
	VULKAN_FUNCTION(vkEnumerateInstanceLayerProperties)\
	VULKAN_FUNCTION(vkCreateInstance)\

#define VULKAN_INSTANCE_FUNCTION_LIST_NO_DEBUG \
	VULKAN_FUNCTION(vkGetDeviceProcAddr)\
	VULKAN_FUNCTION(vkDestroyInstance)\
	VULKAN_FUNCTION(vkEnumeratePhysicalDevices)\
	VULKAN_FUNCTION(vkGetPhysicalDeviceProperties)\
	VULKAN_FUNCTION(vkGetPhysicalDeviceSurfaceFormatsKHR)\
	VULKAN_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties)\
	VULKAN_FUNCTION(vkGetPhysicalDeviceSurfaceSupportKHR)\
	VULKAN_FUNCTION(vkGetPhysicalDeviceSurfaceCapabilitiesKHR)\
	VULKAN_FUNCTION(vkGetPhysicalDeviceSurfacePresentModesKHR)\
	VULKAN_FUNCTION(vkEnumerateDeviceExtensionProperties)\
	VULKAN_FUNCTION(vkDestroySurfaceKHR)\
	VULKAN_FUNCTION(vkCreateDevice)\

#define VULKAN_INSTANCE_FUNCTION_LIST_DEBUG \
	VULKAN_FUNCTION(vkCreateDebugUtilsMessengerEXT)\
	VULKAN_FUNCTION(vkDestroyDebugUtilsMessengerEXT)\

#ifdef MINEC_CLIENT_DEBUG_LOG

#define VULKAN_INSTANCE_FUNCTION_LIST \
	VULKAN_INSTANCE_FUNCTION_LIST_DEBUG \
	VULKAN_INSTANCE_FUNCTION_LIST_NO_DEBUG \

#else

#define VULKAN_INSTANCE_FUNCTION_LIST \
	VULKAN_INSTANCE_FUNCTION_LIST_NO_DEBUG \

#endif

#define VULKAN_DEVICE_FUNCTION_LIST \
	VULKAN_FUNCTION(vkDestroyDevice)\
	VULKAN_FUNCTION(vkDeviceWaitIdle)\
	VULKAN_FUNCTION(vkGetDeviceQueue)\
	VULKAN_FUNCTION(vkQueueWaitIdle)\
	VULKAN_FUNCTION(vkCreateSwapchainKHR)\
	VULKAN_FUNCTION(vkDestroySwapchainKHR)\
	VULKAN_FUNCTION(vkGetSwapchainImagesKHR)\
	VULKAN_FUNCTION(vkCreateImageView)\
	VULKAN_FUNCTION(vkDestroyImageView)\
	VULKAN_FUNCTION(vkCreateImage)\
	VULKAN_FUNCTION(vkDestroyImage)\
	VULKAN_FUNCTION(vkCreateFramebuffer)\
	VULKAN_FUNCTION(vkDestroyFramebuffer)\

struct renderer_backend_vulkan
{

	struct 
	{
#define VULKAN_FUNCTION(name) PFN_##name name;
		VULKAN_FUNCTION(vkGetInstanceProcAddr)\
		VULKAN_PRE_INSTANCE_FUNCTION_LIST \
		VULKAN_INSTANCE_FUNCTION_LIST \
		VULKAN_DEVICE_FUNCTION_LIST
#undef VULKAN_FUNCTION
	} func;

	VkInstance instance;
	VkDebugUtilsMessengerEXT debug_messenger;

	VkPhysicalDevice physical_devices[RENDERER_MAX_BACKEND_DEVICE_COUNT];

	VkDevice device;
};
struct minec_client;

struct renderer_backend_info* renderer_backend_vulkan_get_info(
	struct minec_client* client
);

uint32_t renderer_backend_vulkan_base_create(
	struct minec_client* client,
	cwindow_context* window_context,
	cwindow* window,
	struct renderer_backend_device_infos* device_infos
);

void renderer_backend_vulkan_base_destroy(
	struct minec_client* client
);

uint32_t renderer_backend_vulkan_device_create(
	struct minec_client* client,
	uint32_t device_index
);

void renderer_backend_vulkan_device_destroy(
	struct minec_client* client
);

uint32_t renderer_backend_vulkan_swapchain_create(
	struct minec_client* client,
	uint32_t width,
	uint32_t height,
	bool vsync,
	bool triple_buffering
);

void renderer_backend_vulkan_swapchain_destroy(
	struct minec_client* client
);

uint32_t renderer_backend_vulkan_frame_start(
	struct minec_client* client
);

uint32_t renderer_backend_vulkan_frame_submit(
	struct minec_client* client
);

#endif