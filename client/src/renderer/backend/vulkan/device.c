#include "backend_vulkan.h"

uint32_t renderer_backend_vulkan_device_create(struct minec_client* client, void** backend_base, void** backend_device, uint32_t device_index, uint32_t fps)
{
    uint32_t return_value = MINEC_CLIENT_SUCCESS;

    bool
        device_memory = false,
        surface_formats_memory = false,
        queue_family_properties_memory = false,
        device_created = false
    ;

    struct renderer_backend_vulkan_base* base = *backend_base;
    struct renderer_backend_vulkan_device* device;

    if (base->physical_device_properties[device_index].apiVersion < VK_API_VERSION_1_0)
    {
        renderer_backend_vulkan_log(client, "vulkan device version is below 1.0");
        return_value = MINEC_CLIENT_ERROR;
    }

    if ((device = s_alloc(client->static_alloc, sizeof(struct renderer_backend_vulkan_device))) == NULL)
        return_value = MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
    else
        device_memory = true;

    VkSurfaceFormatKHR* surface_formats;
    uint32_t surface_format_count;

    VkQueueFamilyProperties* queue_family_properties;
    uint32_t queue_family_properties_count;

    if (return_value == MINEC_CLIENT_SUCCESS)
    {
        if (base->func.vkGetPhysicalDeviceSurfaceFormatsKHR(base->physical_devices[device_index], base->surface, &surface_format_count, NULL) != VK_SUCCESS)
        {
            renderer_backend_vulkan_log(client, "vkGetPhysicalDeviceSurfaceFormatsKHR failed");
            return_value = MINEC_CLIENT_ERROR;
        }
        else
        {
            if ((surface_formats = s_alloc(client->dynamic_alloc, sizeof(VkSurfaceFormatKHR) * surface_format_count)) == NULL)
                return_value = MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
            else
            {
                surface_formats_memory = true;

                if (base->func.vkGetPhysicalDeviceSurfaceFormatsKHR(base->physical_devices[device_index], base->surface, &surface_format_count, surface_formats) != VK_SUCCESS)
                {
                    renderer_backend_vulkan_log(client, "vkGetPhysicalDeviceSurfaceFormatsKHR failed");
                    return_value = MINEC_CLIENT_ERROR;
                }
            }
        }
    }

    if (return_value == MINEC_CLIENT_SUCCESS)
    {
        base->func.vkGetPhysicalDeviceQueueFamilyProperties(base->physical_devices[device_index], &queue_family_properties_count, NULL);

        if ((queue_family_properties = s_alloc(client->dynamic_alloc, sizeof(VkQueueFamilyProperties) * queue_family_properties_count)) == NULL)
            return_value = MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
        else
        {
            queue_family_properties_memory = true;
            base->func.vkGetPhysicalDeviceQueueFamilyProperties(base->physical_devices[device_index], &queue_family_properties_count, queue_family_properties);
        }
    }

    if (return_value == MINEC_CLIENT_SUCCESS)
    {
        if (surface_format_count == 1 && surface_formats[0].format == VK_FORMAT_UNDEFINED)
        {
            device->surface_format.format = VK_FORMAT_B8G8R8A8_UNORM;
            device->surface_format.colorSpace = surface_formats[0].colorSpace;
        }
        else
        {
            bool found = false;

            for (uint32_t i = 0; i < surface_format_count && found == false; i++) if (surface_formats[i].format == VK_FORMAT_B8G8R8A8_UNORM)
            {
                device->surface_format = surface_formats[i];
                found = true;
                break;
            }

            for (uint32_t i = 0; i < surface_format_count && found == false; i++) if (surface_formats[i].format == VK_FORMAT_R8G8B8A8_UNORM)
            {
                device->surface_format = surface_formats[i];
                found = true;
                break;
            }

            if (found == false)
            {
                renderer_backend_vulkan_log(client, "could not find suitable surface format");
                return_value = MINEC_CLIENT_ERROR;
            }
        }

    }

    if (return_value == MINEC_CLIENT_SUCCESS) for (uint32_t i = 0; ; i++) {

        if (i == queue_family_properties_count)
        {
            renderer_backend_vulkan_log(client, "could not find suitable queue family");
            return_value = MINEC_CLIENT_ERROR;
            break;
        }
        else if ((queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)) {

            VkBool32 surface_support = VK_FALSE;
            base->func.vkGetPhysicalDeviceSurfaceSupportKHR(base->physical_devices[device_index], i, base->surface, &surface_support);

            if (surface_support == VK_TRUE)
            {
                device->queue_family_index = i;
                break;
            }
        }
    }
    
    uint8_t* device_extentions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
        VK_KHR_MAINTENANCE3_EXTENSION_NAME
    };

    if (return_value == MINEC_CLIENT_SUCCESS)
    {
        VkExtensionProperties* extensions;
        uint32_t extension_count;

        if (base->func.vkEnumerateDeviceExtensionProperties(base->physical_devices[device_index], NULL, &extension_count, NULL) != VK_SUCCESS)
        {
            renderer_backend_vulkan_log(client, "vkEnumerateDeviceExtensionProperties failed");
            return_value = MINEC_CLIENT_ERROR;
        }
        else
        {
            if ((extensions = s_alloc(client->dynamic_alloc, sizeof(VkExtensionProperties) * extension_count)) == NULL)
                return_value = MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
            else if (base->func.vkEnumerateDeviceExtensionProperties(base->physical_devices[device_index], NULL, &extension_count, extensions) != VK_SUCCESS)
            {
                renderer_backend_vulkan_log(client, "vkEnumerateDeviceExtensionProperties failed");
                s_free(client->dynamic_alloc, extensions);
                return_value = MINEC_CLIENT_ERROR;
            }
            else
            {
                for (uint32_t i = 0; i < sizeof(device_extentions) / sizeof(device_extentions[0]) && return_value == MINEC_CLIENT_SUCCESS; i++)
                {
                    bool found = false;
                    for (uint32_t j = 0; j < extension_count && found == false; j++) 
                        if (strcmp(device_extentions[i], extensions[j].extensionName) == 0) 
                            found = true;

                    if (found == false)
                    {
                        renderer_backend_vulkan_log(client, "Required VkDevice extension '%s' not supported", device_extentions[i]);
                        return_value = MINEC_CLIENT_ERROR;
                        break;
                    }
                }
                s_free(client->dynamic_alloc, extensions);
            }
        }
    }

    if (return_value == MINEC_CLIENT_SUCCESS)
    {
        float queue_priority = 1.f;

        VkDeviceQueueCreateInfo queue_info = { 0 };
        queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info.queueFamilyIndex = device->queue_family_index;
        queue_info.queueCount = 1;
        queue_info.pQueuePriorities = &queue_priority;

        char* device_extentions[] = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
            VK_KHR_MAINTENANCE3_EXTENSION_NAME,
        };

        VkPhysicalDeviceVulkan12Features features12 = { 0 };
        features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        features12.runtimeDescriptorArray = VK_TRUE;
        features12.descriptorIndexing = VK_TRUE;

        VkPhysicalDeviceFeatures enabled_features = { 0 };
        enabled_features.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo device_info = { 0 };
        device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_info.pQueueCreateInfos = &queue_info;
        device_info.queueCreateInfoCount = 1;
        device_info.ppEnabledExtensionNames = (const char* const*)device_extentions;
        device_info.enabledExtensionCount = 3;
        device_info.pEnabledFeatures = &enabled_features;
        device_info.pNext = &features12;

        if (base->func.vkCreateDevice(base->physical_devices[device_index], &device_info, 0, &device->device) != VK_SUCCESS)
        {
            renderer_backend_vulkan_log(client, "vkCreateDevice failed");
            return_value = MINEC_CLIENT_ERROR;
        }
    }

    {
        void** functions[] = {
            (void**)&device->func.vkGetDeviceQueue
        };

        uint8_t* function_names[] = {
            "vkGetDeviceQueue"
        };

        for (uint32_t i = 0; i < sizeof(functions) / sizeof(functions[0]) && return_value == MINEC_CLIENT_SUCCESS; i++)
        {
            if ((*functions[i] = base->func.vkGetDeviceProcAddr(device->device, function_names[i])) == NULL)
            {
                renderer_backend_vulkan_log(client, "Failed to retrieve '%s'", function_names[i]);
                return_value = MINEC_CLIENT_ERROR;
            }
        }
    }

    if (return_value == MINEC_CLIENT_SUCCESS)
    {
        device->func.vkGetDeviceQueue(device->device, device->queue_family_index, 0, &device->queue);
    }

    if (return_value == MINEC_CLIENT_SUCCESS)
    {
        *backend_device = device;
    }

    if (return_value != MINEC_CLIENT_SUCCESS && device_created) base->func.vkDestroyDevice(device->device, 0);
    if (queue_family_properties_memory) s_free(client->dynamic_alloc, queue_family_properties);
    if (surface_formats_memory) s_free(client->dynamic_alloc, surface_formats);
    if (return_value != MINEC_CLIENT_SUCCESS && device_memory) s_free(client->static_alloc, device);

    return return_value;
}

void renderer_backend_vulkan_device_destroy(struct minec_client* client, void** backend_base, void** backend_device)
{
    struct renderer_backend_vulkan_base* base = *backend_base;
    struct renderer_backend_vulkan_device* device = *backend_device;

    base->func.vkDestroyDevice(device->device, 0);
    s_free(client->static_alloc, device);
}

void renderer_backend_vulkan_set_fps(struct minec_client* client, uint32_t fps)
{

}