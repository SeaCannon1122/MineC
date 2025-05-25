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

#define VULKAN_SWAPCHAIN_IMAGE_COUNT 2

struct renderer_backend_vulkan_base
{
	struct
	{
		PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
		PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;

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
		PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
		PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
		PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
		PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
		PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
		PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;

		PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
		PFN_vkCreateDevice vkCreateDevice;
		PFN_vkDestroyDevice vkDestroyDevice;

	} func;

	VkInstance instance;
#ifdef MINEC_CLIENT_DEBUG
	VkDebugUtilsMessengerEXT debug_messenger;
#endif
	VkPhysicalDevice* physical_devices;
	uint8_t** backend_device_infos;
	uint32_t physical_device_count;

	VkSurfaceKHR surface;
};



struct renderer_backend_vulkan_device
{
	struct
	{
		PFN_vkDeviceWaitIdle vkDeviceWaitIdle;

		PFN_vkGetDeviceQueue vkGetDeviceQueue;
		PFN_vkQueueWaitIdle vkQueueWaitIdle;

		PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
		PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
		PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;

		PFN_vkCreateImageView vkCreateImageView;
		PFN_vkDestroyImageView vkDestroyImageView;
		PFN_vkCreateImage vkCreateImage;
		PFN_vkDestroyImage vkDestroyImage;
		PFN_vkCreateFramebuffer vkCreateFramebuffer;
		PFN_vkDestroyFramebuffer vkDestroyFramebuffer;

	} func;

	VkDevice device;
	uint32_t physical_device_index;
	VkPhysicalDeviceProperties physical_device_properties;

	VkSurfaceFormatKHR surface_format;
	VkSurfaceCapabilitiesKHR surface_capabilities;

	VkQueue graphics_queue;
	uint32_t graphics_queue_family_index;
	VkQueue transfer_queue;
	uint32_t transfer_queue_family_index;

	struct
	{
		bool created;

		bool immediate_support;
		bool mailbox_support;

		VkSwapchainKHR swapchain;
		VkPresentModeKHR present_mode;

		VkImage images[VULKAN_SWAPCHAIN_IMAGE_COUNT];
		VkImageView image_views[VULKAN_SWAPCHAIN_IMAGE_COUNT];
	} swapchain;
};

struct renderer_backend_vulkan_pipelines_resources
{
	struct
	{
		bool usable;
	} pixelchar_renderer;
};

#endif
