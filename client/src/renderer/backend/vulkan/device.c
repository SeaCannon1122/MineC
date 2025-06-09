#include "backend_vulkan.h"

uint32_t _swapchain_create(struct minec_client* client, struct renderer_backend_vulkan_base* base, struct renderer_backend_vulkan_device* device)
{
    uint32_t result = MINEC_CLIENT_SUCCESS;

    bool
        swapchain_created = false
    ;

    if (device->surface_capabilities.minImageCount > VULKAN_SWAPCHAIN_IMAGE_COUNT || device->surface_capabilities.maxImageCount < VULKAN_SWAPCHAIN_IMAGE_COUNT)
    {
        renderer_backend_vulkan_log(client, "exactly two swapchain images not supported");
        return MINEC_CLIENT_ERROR;
    }

    VkSwapchainCreateInfoKHR swapchain_info = { 0 };
    swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_info.surface = base->surface;
    swapchain_info.minImageCount = VULKAN_SWAPCHAIN_IMAGE_COUNT;
    swapchain_info.imageFormat = device->surface_format.format;
    swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_info.preTransform = device->surface_capabilities.currentTransform;
    swapchain_info.imageExtent = device->surface_capabilities.currentExtent;
    swapchain_info.imageArrayLayers = 1;
    swapchain_info.presentMode = device->swapchain.present_mode;

    if (device->func.vkCreateSwapchainKHR(device->device, &swapchain_info, 0, &device->swapchain.swapchain) != VK_SUCCESS)
    {
        renderer_backend_vulkan_log(client, "vkCreateSwapchainKHR failed");
        result = MINEC_CLIENT_ERROR;
    }
    else swapchain_created = true;

    if (result == MINEC_CLIENT_SUCCESS)
    {
        uint32_t swapchain_image_count = VULKAN_SWAPCHAIN_IMAGE_COUNT;
        if (device->func.vkGetSwapchainImagesKHR(device->device, device->swapchain.swapchain, &swapchain_image_count, device->swapchain.images) != VK_SUCCESS)
        {
            renderer_backend_vulkan_log(client, "vkGetSwapchainImagesKHR failed");
            result = MINEC_CLIENT_ERROR;
        }
    }

    if (result == MINEC_CLIENT_SUCCESS)
    {
        VkImageViewCreateInfo image_view_info = { 0 };
        image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_info.format = device->surface_format.format;
        image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_info.subresourceRange.layerCount = 1;
        image_view_info.subresourceRange.levelCount = 1;

        for (uint32_t i = 0; i < VULKAN_SWAPCHAIN_IMAGE_COUNT; i++) {

            image_view_info.image = device->swapchain.images[i];
            if (device->func.vkCreateImageView(device->device, &image_view_info, 0, &device->swapchain.image_views[i]) != VK_SUCCESS)
            {
                for (uint32_t j = 0; j < i; j++) device->func.vkDestroyImageView(device->device, device->swapchain.image_views[j], 0);

                renderer_backend_vulkan_log(client, "vkCreateImageView failed");
                result = MINEC_CLIENT_ERROR;
            }
        }
    }

    if (result != MINEC_CLIENT_SUCCESS && swapchain_created) device->func.vkDestroySwapchainKHR(device->device, device->swapchain.swapchain, 0);

    return result;
}

void _swapchain_destroy(struct minec_client* client, struct renderer_backend_vulkan_base* base, struct renderer_backend_vulkan_device* device)
{
    for (uint32_t i = 0; i < VULKAN_SWAPCHAIN_IMAGE_COUNT; i++) device->func.vkDestroyImageView(device->device, device->swapchain.image_views[i], 0);
    device->func.vkDestroySwapchainKHR(device->device, device->swapchain.swapchain, 0);
}

uint32_t renderer_backend_vulkan_device_create(struct minec_client* client, void** backend_base, void** backend_device, uint32_t device_index, uint32_t fps)
{
    uint32_t result = MINEC_CLIENT_SUCCESS;

    bool
        device_memory = false,
        surface_formats_memory = false,
        queue_family_properties_memory = false,
        present_modes_memory = false,
        device_created = false,
        swapchain_created = false
    ;

    struct renderer_backend_vulkan_base* base = *backend_base;
    struct renderer_backend_vulkan_device* device;

    if ((device = s_alloc(client->static_alloc, sizeof(struct renderer_backend_vulkan_device))) == NULL)
        result = MINEC_CLIENT_CRITICAL_ERROR;
    else
        device_memory = true;

    device->physical_device_index = device_index;

    if (result == MINEC_CLIENT_SUCCESS)
    {
        base->func.vkGetPhysicalDeviceProperties(base->physical_devices[device_index], &device->physical_device_properties);

        if (device->physical_device_properties.apiVersion < VK_API_VERSION_1_0)
        {
            renderer_backend_vulkan_log(client, "vulkan device version is below 1.0");
            result = MINEC_CLIENT_ERROR;
        }
    }
    
    if (result == MINEC_CLIENT_SUCCESS)
    {
        if (base->func.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(base->physical_devices[device_index], base->surface, &device->surface_capabilities) != VK_SUCCESS)
        {
            renderer_backend_vulkan_log(client, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed");
            result = MINEC_CLIENT_ERROR;
        }
    }

    VkSurfaceFormatKHR* surface_formats;
    uint32_t surface_format_count;

    VkQueueFamilyProperties* queue_family_properties;
    uint32_t queue_family_properties_count;

    VkPresentModeKHR* present_modes;
    uint32_t present_mode_count;

    if (result == MINEC_CLIENT_SUCCESS)
    {
        if (base->func.vkGetPhysicalDeviceSurfaceFormatsKHR(base->physical_devices[device_index], base->surface, &surface_format_count, NULL) != VK_SUCCESS)
        {
            renderer_backend_vulkan_log(client, "vkGetPhysicalDeviceSurfaceFormatsKHR failed");
            result = MINEC_CLIENT_ERROR;
        }
        else
        {
            if ((surface_formats = s_alloc(client->dynamic_alloc, sizeof(VkSurfaceFormatKHR) * surface_format_count)) == NULL)
                result = MINEC_CLIENT_CRITICAL_ERROR;
            else
            {
                surface_formats_memory = true;

                if (base->func.vkGetPhysicalDeviceSurfaceFormatsKHR(base->physical_devices[device_index], base->surface, &surface_format_count, surface_formats) != VK_SUCCESS)
                {
                    renderer_backend_vulkan_log(client, "vkGetPhysicalDeviceSurfaceFormatsKHR failed");
                    result = MINEC_CLIENT_ERROR;
                }
            }
        }
    }

    if (result == MINEC_CLIENT_SUCCESS)
    {
        base->func.vkGetPhysicalDeviceQueueFamilyProperties(base->physical_devices[device_index], &queue_family_properties_count, NULL);

        if ((queue_family_properties = s_alloc(client->dynamic_alloc, sizeof(VkQueueFamilyProperties) * queue_family_properties_count)) == NULL)
            result = MINEC_CLIENT_CRITICAL_ERROR;
        else
        {
            queue_family_properties_memory = true;
            base->func.vkGetPhysicalDeviceQueueFamilyProperties(base->physical_devices[device_index], &queue_family_properties_count, queue_family_properties);
        }
    }

    if (result == MINEC_CLIENT_SUCCESS)
    {
        if (base->func.vkGetPhysicalDeviceSurfacePresentModesKHR(base->physical_devices[device_index], base->surface, &present_mode_count, NULL) != VK_SUCCESS)
        {
            renderer_backend_vulkan_log(client, "vkGetPhysicalDeviceSurfacePresentModesKHR failed");
            result = MINEC_CLIENT_ERROR;
        }
        else
        {
            if ((present_modes = s_alloc(client->dynamic_alloc, sizeof(VkPresentModeKHR) * present_mode_count)) == NULL)
                result = MINEC_CLIENT_CRITICAL_ERROR;
            else
            {
                present_modes_memory = true;

                if (base->func.vkGetPhysicalDeviceSurfacePresentModesKHR(base->physical_devices[device_index], base->surface, &present_mode_count, present_modes) != VK_SUCCESS)
                {
                    renderer_backend_vulkan_log(client, "vkGetPhysicalDeviceSurfacePresentModesKHR failed");
                    result = MINEC_CLIENT_ERROR;
                }
            }
        }
    }

    if (result == MINEC_CLIENT_SUCCESS)
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
                result = MINEC_CLIENT_ERROR;
            }
        }

    }

    if (result == MINEC_CLIENT_SUCCESS)
    {
        bool graphics_queue_found = false;

        for (uint32_t i = 0; i < queue_family_properties_count; i++) {
            
            if ((queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)) {

                VkBool32 surface_support = VK_FALSE;
                base->func.vkGetPhysicalDeviceSurfaceSupportKHR(base->physical_devices[device_index], i, base->surface, &surface_support);

                if (surface_support == VK_TRUE)
                {
                    device->graphics_queue_family_index = i;
                    graphics_queue_found = true;
                    break;
                }
            }
        }

        if (graphics_queue_found == false)
        {
            renderer_backend_vulkan_log(client, "could not find suitable queue family for graphics queue");
            result = MINEC_CLIENT_ERROR;
        }
        else {

            for (uint32_t i = 0; ; i++) {

                if (i == queue_family_properties_count)
                {
                    if (queue_family_properties[device->graphics_queue_family_index].queueCount < 2)
                    {
                        renderer_backend_vulkan_log(client, "could not find suitable queue family for transfer queue");
                        result = MINEC_CLIENT_ERROR;
                    }
                    else device->transfer_queue_family_index = device->graphics_queue_family_index;
                    
                    break;
                }
                else if ((queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0 && (queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT))
                {
                    device->transfer_queue_family_index = i;
                    break;
                }
            }

        }
    }

    if (result == MINEC_CLIENT_SUCCESS) {
    }
    
    if (result == MINEC_CLIENT_SUCCESS)
    {
        device->swapchain.immediate_support = false;
        device->swapchain.mailbox_support = false;

        for (uint32_t i = 0; i < present_mode_count; i++)
        {
            if (present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) device->swapchain.immediate_support = true;
            else if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) device->swapchain.mailbox_support = true;
        }
    }

    uint8_t* device_extentions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
        VK_KHR_MAINTENANCE3_EXTENSION_NAME
    };

    if (result == MINEC_CLIENT_SUCCESS)
    {
        VkExtensionProperties* extensions;
        uint32_t extension_count;

        if (base->func.vkEnumerateDeviceExtensionProperties(base->physical_devices[device_index], NULL, &extension_count, NULL) != VK_SUCCESS)
        {
            renderer_backend_vulkan_log(client, "vkEnumerateDeviceExtensionProperties failed");
            result = MINEC_CLIENT_ERROR;
        }
        else
        {
            if ((extensions = s_alloc(client->dynamic_alloc, sizeof(VkExtensionProperties) * extension_count)) == NULL)
                result = MINEC_CLIENT_CRITICAL_ERROR;
            else if (base->func.vkEnumerateDeviceExtensionProperties(base->physical_devices[device_index], NULL, &extension_count, extensions) != VK_SUCCESS)
            {
                renderer_backend_vulkan_log(client, "vkEnumerateDeviceExtensionProperties failed");
                s_free(client->dynamic_alloc, extensions);
                result = MINEC_CLIENT_ERROR;
            }
            else
            {
                for (uint32_t i = 0; i < sizeof(device_extentions) / sizeof(device_extentions[0]) && result == MINEC_CLIENT_SUCCESS; i++)
                {
                    bool found = false;
                    for (uint32_t j = 0; j < extension_count && found == false; j++) 
                        if (strcmp(device_extentions[i], extensions[j].extensionName) == 0) 
                            found = true;

                    if (found == false)
                    {
                        renderer_backend_vulkan_log(client, "Required VkDevice extension '%s' not supported", device_extentions[i]);
                        result = MINEC_CLIENT_ERROR;
                        break;
                    }
                }
                s_free(client->dynamic_alloc, extensions);
            }
        }
    }

    if (result == MINEC_CLIENT_SUCCESS)
    {
        float queue_priorities[] = { 1.f, 1.f };

        VkDeviceQueueCreateInfo graphics_queue_info = { 0 };
        graphics_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        graphics_queue_info.queueFamilyIndex = device->graphics_queue_family_index;
        graphics_queue_info.pQueuePriorities = queue_priorities;
        graphics_queue_info.queueCount = 1;

        VkDeviceQueueCreateInfo transfer_queue_info = { 0 };
        transfer_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        transfer_queue_info.queueFamilyIndex = device->transfer_queue_family_index;
        transfer_queue_info.pQueuePriorities = queue_priorities;
        transfer_queue_info.queueCount = 1;

        VkDeviceQueueCreateInfo queue_infos[] = {
            graphics_queue_info,
            transfer_queue_info
        };

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
        device_info.pQueueCreateInfos = queue_infos;

        if (device->transfer_queue_family_index == device->graphics_queue_family_index)
        {
            queue_infos[0].queueCount = 2;
            device_info.queueCreateInfoCount = 1;
        }
        else
        {
            device_info.queueCreateInfoCount = 2;
        }
        
        device_info.ppEnabledExtensionNames = (const char* const*)device_extentions;
        device_info.enabledExtensionCount = 3;
        device_info.pEnabledFeatures = &enabled_features;
        device_info.pNext = &features12;

        if (base->func.vkCreateDevice(base->physical_devices[device_index], &device_info, 0, &device->device) != VK_SUCCESS)
        {
            renderer_backend_vulkan_log(client, "vkCreateDevice failed");
            result = MINEC_CLIENT_ERROR;
        }
    }

    {
        void** functions[] = {
            (void**)&device->func.vkDeviceWaitIdle,
            (void**)&device->func.vkGetDeviceQueue,
            (void**)&device->func.vkQueueWaitIdle,
            (void**)&device->func.vkCreateSwapchainKHR,
            (void**)&device->func.vkDestroySwapchainKHR,
            (void**)&device->func.vkGetSwapchainImagesKHR,
            (void**)&device->func.vkCreateImageView,
            (void**)&device->func.vkDestroyImageView,
            (void**)&device->func.vkCreateImage,
            (void**)&device->func.vkDestroyImage,
            (void**)&device->func.vkCreateFramebuffer,
            (void**)&device->func.vkDestroyFramebuffer,
        };

        uint8_t* function_names[] = {
            "vkDeviceWaitIdle",
            "vkGetDeviceQueue",
            "vkQueueWaitIdle",
            "vkCreateSwapchainKHR",
            "vkDestroySwapchainKHR",
            "vkGetSwapchainImagesKHR",
            "vkCreateImageView",
            "vkDestroyImageView",
            "vkCreateImage",
            "vkDestroyImage",
            "vkCreateFramebuffer",
            "vkDestroyFramebuffer",
        };

        for (uint32_t i = 0; i < sizeof(functions) / sizeof(functions[0]) && result == MINEC_CLIENT_SUCCESS; i++)
        {
            if ((*functions[i] = base->func.vkGetDeviceProcAddr(device->device, function_names[i])) == NULL)
            {
                renderer_backend_vulkan_log(client, "Failed to retrieve '%s'", function_names[i]);
                result = MINEC_CLIENT_ERROR;
            }
        }
    }

    if (result == MINEC_CLIENT_SUCCESS)
    {
        device->swapchain.present_mode = VK_PRESENT_MODE_FIFO_KHR;
        if (fps != 0 && device->swapchain.immediate_support == true) device->swapchain.present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;

        result = _swapchain_create(client, base, device);
        device->swapchain.created = true;
    }

    if (result == MINEC_CLIENT_SUCCESS)
    {
        device->func.vkGetDeviceQueue(device->device, device->graphics_queue_family_index, 0, &device->graphics_queue);

        if (device->transfer_queue_family_index == device->graphics_queue_family_index)
        {
            device->func.vkGetDeviceQueue(device->device, device->graphics_queue_family_index, 1, &device->transfer_queue);
        }
        else
        {
            device->func.vkGetDeviceQueue(device->device, device->transfer_queue_family_index, 0, &device->transfer_queue);
        }
    }

    if (result == MINEC_CLIENT_SUCCESS)
    {
        *backend_device = device;
    }

    if (result != MINEC_CLIENT_SUCCESS && device_created) base->func.vkDestroyDevice(device->device, 0);
    if (present_modes_memory) s_free(client->dynamic_alloc, present_modes);
    if (queue_family_properties_memory) s_free(client->dynamic_alloc, queue_family_properties);
    if (surface_formats_memory) s_free(client->dynamic_alloc, surface_formats);
    if (result != MINEC_CLIENT_SUCCESS && device_memory) s_free(client->static_alloc, device);

    return result;
}

void renderer_backend_vulkan_device_destroy(struct minec_client* client, void** backend_base, void** backend_device)
{
    struct renderer_backend_vulkan_base* base = *backend_base;
    struct renderer_backend_vulkan_device* device = *backend_device;

    if (device->swapchain.created == true) _swapchain_destroy(client, base, device);
    base->func.vkDestroyDevice(device->device, 0);
    s_free(client->static_alloc, device);
}

uint32_t renderer_backend_vulkan_set_fps(struct minec_client* client, uint32_t fps)
{
    struct renderer_backend_vulkan_base* base = client->renderer.backend.base.base;
    struct renderer_backend_vulkan_device* device = client->renderer.backend.device.device;;
}