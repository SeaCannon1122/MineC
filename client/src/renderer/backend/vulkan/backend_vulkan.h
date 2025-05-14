#pragma once

#ifndef MINEC_CLIENT_RENDERER_BACKEND_VULKAN_BACKEND_VULKAN_H
#define MINEC_CLIENT_RENDERER_BACKEND_VULKAN_BACKEND_VULKAN_H

#include "../backend_internal.h"

static void renderer_backend_vulkan_log(struct minec_client* client, uint8_t* message, ...)
{
	va_list args;
	va_start(args, message);
	minec_client_log_v(client, "[RENDERER][VULKAN]", message, args);
	va_end(args);
}

struct renderer_backend_vulkan_base
{
	struct
	{
		void* libarary_handle;

		PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;

		PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
		PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
#ifdef MINEC_CLIENT_DEBUG
		PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
		PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
#endif

		PFN_vkCreateInstance vkCreateInstance;
		PFN_vkDestroyInstance vkDestroyInstance;

		PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
		PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
		PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
		PFN_vkCreateDevice vkCreateDevice;
		PFN_vkDestroyDevice vkDestroyDevice;

	} func;

	VkInstance instance;
#ifdef MINEC_CLIENT_DEBUG
	VkDebugUtilsMessengerEXT debug_messenger;
#endif
	VkPhysicalDevice* physical_devices;
	VkPhysicalDeviceProperties* physical_device_properties;
	uint8_t** backend_device_infos;
	uint32_t physical_device_count;

	VkSurfaceKHR surface;
	VkSurfaceCapabilitiesKHR surface_capabilities;
};

struct renderer_backend_vulkan_device
{
	struct
	{
		int a;
	} func;

	VkDevice device;
	VkPhysicalDevice physical_device;
	uint32_t physical_device_index;

	VkQueue queue;
	uint32_t queue_index;

	VkSurfaceFormatKHR surface_format;

	VkSwapchainKHR swapchain;
	uint32_t swapchain_length;
};

struct renderer_backend_vulkan_pipelines_resources
{
	int a;
};

#endif
