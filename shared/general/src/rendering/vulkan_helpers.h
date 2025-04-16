#pragma once

#include "general/platformlib/platform/platform.h"

#define VKCall(call) \
do { \
    VkResult result = (call); \
    if (result != VK_SUCCESS) { \
        printf("Vulkan error in \n    %s \n at %s:%d: %d\n", #call, __FILE__, __LINE__, result); \
		DEBUG_BREAK();\
    } \
} while(0)

uint32_t get_vulkan_version(uint32_t* major, uint32_t* minor, uint32_t* patch);

VkResult new_VkInstance(uint8_t* app_name, uint8_t* engine_name, VkInstance* instance, VkDebugUtilsMessengerEXT* debug_messenger);
VkResult destroy_VkInstance(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger);

VkResult new_VkDevice(VkInstance instance, VkPhysicalDevice* gpu, uint32_t* queue_family_index, float queue_priority, VkDevice* device);

VkResult get_surface_format(VkInstance instance, VkPhysicalDevice gpu, VkFormat format, VkSurfaceFormatKHR* surface_format);

VkResult new_VkShaderModule(VkDevice device, uint8_t* file_path, VkShaderModule* shader_module);

VkPipelineShaderStageCreateInfo shader_stage(VkShaderModule shader_module, VkShaderStageFlagBits flag_bits);
VkResult command_pool_create(VkDevice device, uint32_t queue_index, VkCommandPool* command_pool);
VkResult command_buffer_allocate(VkDevice device, VkCommandPool command_pool, VkCommandBuffer* cmd);
VkResult semaphore_create(VkDevice device, VkSemaphore* semaphore);
VkResult fence_create(VkDevice device, VkFence* fence);