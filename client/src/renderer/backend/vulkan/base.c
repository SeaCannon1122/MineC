#include "backend_vulkan.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
    VkDebugUtilsMessageTypeFlagsEXT msg_flags,
    const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
    void* p_user_data
) {
    printf("--------------------------------------------------------------------------\n\nValidation Error: %s\n\n\n\n", p_callback_data->pMessage);
    return 0;
}

uint32_t renderer_backend_vulkan_base_create(struct minec_client* client, void** backend_base, uint32_t* device_count, uint8_t*** device_infos)
{
    uint32_t return_value = MINEC_CLIENT_SUCCESS;

    bool 
        window_context_initialized = false,
        base_memory = false, 
        library_loaded = false,
        extensions_memory = false,
#ifdef MINEC_CLIENT_DEBUG
        layers_memory = false,
        debug_messenger_created = false,
#endif
        instance_created = false,
        device_memory = false,
        device_properties_memory = false,
        surface_created = false
    ;

    window_init_context(renderer_backend_get_window_context());

    struct renderer_backend_vulkan_base* base;

    if ((base = s_alloc(client->static_alloc, sizeof(struct renderer_backend_vulkan_base))) == NULL)
        return_value = MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
    else 
        base_memory = true;

    if (return_value == MINEC_CLIENT_SUCCESS)
    {
        if ((base->func.libarary_handle = dynamic_library_load(MINEC_CLIENT_VULKAN_LIBRARY_NAME, true)) == NULL)
        {
            renderer_backend_vulkan_log(client, "Failed to load '%s'", MINEC_CLIENT_VULKAN_LIBRARY_NAME);
            return_value = MINEC_CLIENT_ERROR;
        }
        else 
            library_loaded = true;
    }

    if (return_value == MINEC_CLIENT_SUCCESS) {

        if ((base->func.vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dynamic_library_get_function(base->func.libarary_handle, "vkGetInstanceProcAddr")) == NULL)
        {
            renderer_backend_vulkan_log(client, "Failed to retrieve 'vkGetInstanceProcAddr'");
            return_value = MINEC_CLIENT_ERROR;
        }

        void** functions[] = {
            (void**) &base->func.vkEnumerateInstanceExtensionProperties,
            (void**) &base->func.vkEnumerateInstanceLayerProperties,
            (void**) &base->func.vkCreateInstance,
        };

        uint8_t* function_names[] = {
            "vkEnumerateInstanceExtensionProperties",
            "vkEnumerateInstanceLayerProperties",
            "vkCreateInstance",
        };

        for (uint32_t i = 0; i < sizeof(functions) / sizeof(functions[0]) && return_value == MINEC_CLIENT_SUCCESS; i++)
        {
            if ((*functions[i] = base->func.vkGetInstanceProcAddr(NULL, function_names[i])) == NULL)
            {
                renderer_backend_vulkan_log(client, "Failed to retrieve '%s'", function_names[i]);
                return_value = MINEC_CLIENT_ERROR;
            }
        }
    }

    uint8_t* instance_extensions[] = {
        window_get_VK_KHR_PLATFORM_SURFACE_EXTENSION_NAME(),
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    };

    uint32_t extension_count;
    VkExtensionProperties* extensions;

    if (return_value == MINEC_CLIENT_SUCCESS) if (base->func.vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL) != VK_SUCCESS)
    {
        renderer_backend_vulkan_log(client, "vkEnumerateInstanceExtensionProperties failed");
        return_value = MINEC_CLIENT_ERROR;
    }

    if (return_value == MINEC_CLIENT_SUCCESS)
    {
        if ((extensions = s_alloc(client->dynamic_alloc, sizeof(VkExtensionProperties) * extension_count + 8)) == NULL)
            return_value = MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
        else 
            extensions_memory = true;
    }

    if (return_value == MINEC_CLIENT_SUCCESS) if (base->func.vkEnumerateInstanceExtensionProperties(NULL, &extension_count, extensions) != VK_SUCCESS)
    {
        renderer_backend_vulkan_log(client, "vkEnumerateInstanceExtensionProperties failed");
        return_value = MINEC_CLIENT_ERROR;
    }

    for (uint32_t i = 0; i < sizeof(instance_extensions) / sizeof(instance_extensions[0]) && return_value == MINEC_CLIENT_SUCCESS; i++)
    {
        bool extension_support = false;
        for (uint32_t j = 0; j < extension_count && extension_support == false; j++)
        {
            if (strcmp(extensions[j].extensionName, instance_extensions[i]) == 0) 
                extension_support = true;
        }
        if (extension_support == false)
        {
            renderer_backend_vulkan_log(client, "Required VkInstance extension '%s' not supported", instance_extensions[i]);
            return_value = MINEC_CLIENT_ERROR;
        }
    }


#ifdef MINEC_CLIENT_DEBUG

    uint8_t* instance_layers[] = {
        "VK_LAYER_KHRONOS_validation"
    };

    uint32_t layer_count;
    VkLayerProperties* layers;

    if (return_value == MINEC_CLIENT_SUCCESS) if (base->func.vkEnumerateInstanceLayerProperties(&layer_count, NULL) != VK_SUCCESS)
    {
        renderer_backend_vulkan_log(client, "vkEnumerateInstanceLayerProperties failed");
        return_value = MINEC_CLIENT_ERROR;
    }

    if (return_value == MINEC_CLIENT_SUCCESS)
    {
        if ((layers = s_alloc(client->dynamic_alloc, sizeof(VkLayerProperties) * layer_count + 8)) == NULL)
            return_value = MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
        else
            layers_memory = true;
    }

    if (return_value == MINEC_CLIENT_SUCCESS) if (base->func.vkEnumerateInstanceLayerProperties(&layer_count, layers) != VK_SUCCESS)
    {
        renderer_backend_vulkan_log(client, "vkEnumerateInstanceLayerProperties failed");
        return_value = MINEC_CLIENT_ERROR;
    }

    for (uint32_t i = 0; i < sizeof(instance_layers) / sizeof(instance_layers[0]) && return_value == MINEC_CLIENT_SUCCESS; i++)
    {
        bool layer_support = false;
        for (uint32_t j = 0; j < extension_count && layer_support == false; j++)
        {
            if (strcmp(layers[j].layerName, instance_layers[i]) == 0)
                layer_support = true;
        }
        if (layer_support == false)
        {
            renderer_backend_vulkan_log(client, "Required layer extension '%s' not supported", instance_layers[i]);
            return_value = MINEC_CLIENT_ERROR;
        }
    }
#endif

    if (return_value == MINEC_CLIENT_SUCCESS)
    {
        VkApplicationInfo app_info = { 0 };
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "MineC";
        app_info.pEngineName = "MineC_Engine";
        app_info.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo instance_info = { 0 };
        instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_info.pApplicationInfo = &app_info;
        instance_info.ppEnabledExtensionNames = (const char* const*)instance_extensions;
        instance_info.enabledExtensionCount = 4;
#ifdef MINEC_CLIENT_DEBUG
        instance_info.ppEnabledLayerNames = (const char* const*)instance_layers;
        instance_info.enabledLayerCount = 1;
#endif

        if (base->func.vkCreateInstance(&instance_info, 0, &base->instance) != VK_SUCCESS)
        {
            renderer_backend_vulkan_log(client, "vkCreateInstance failed");
            return_value = MINEC_CLIENT_ERROR;
        }
        else 
            instance_created = true;
    }

    {
        void** functions[] = {
            (void**) &base->func.vkGetDeviceProcAddr,
            (void**) &base->func.vkDestroyInstance,
            (void**) &base->func.vkEnumeratePhysicalDevices,
            (void**) &base->func.vkGetPhysicalDeviceProperties,
            (void**) &base->func.vkGetPhysicalDeviceSurfaceFormatsKHR,
            (void**) &base->func.vkGetPhysicalDeviceQueueFamilyProperties,
            (void**) &base->func.vkGetPhysicalDeviceSurfaceSupportKHR,
            (void**) &base->func.vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
            (void**) &base->func.vkEnumerateDeviceExtensionProperties,
            (void**) &base->func.vkDestroySurfaceKHR,
            (void**) &base->func.vkCreateDevice,
            (void**) &base->func.vkDestroyDevice,
#ifdef MINEC_CLIENT_DEBUG
            (void**) &base->func.vkCreateDebugUtilsMessengerEXT,
            (void**) &base->func.vkDestroyDebugUtilsMessengerEXT,
#endif
        };

        uint8_t* function_names[] = {
            "vkGetDeviceProcAddr",
            "vkDestroyInstance",
            "vkEnumeratePhysicalDevices",
            "vkGetPhysicalDeviceProperties",
            "vkGetPhysicalDeviceSurfaceFormatsKHR",
            "vkGetPhysicalDeviceQueueFamilyProperties",
            "vkGetPhysicalDeviceSurfaceSupportKHR",
            "vkGetPhysicalDeviceSurfaceCapabilitiesKHR",
            "vkEnumerateDeviceExtensionProperties",
            "vkDestroySurfaceKHR",
            "vkCreateDevice",
            "vkDestroyDevice",
#ifdef MINEC_CLIENT_DEBUG
            "vkCreateDebugUtilsMessengerEXT",
            "vkDestroyDebugUtilsMessengerEXT",
#endif
        };

        for (uint32_t i = 0; i < sizeof(functions) / sizeof(functions[0]) && return_value == MINEC_CLIENT_SUCCESS; i++)
        {
            if ((*functions[i] = base->func.vkGetInstanceProcAddr(base->instance, function_names[i])) == NULL)
            {
                renderer_backend_vulkan_log(client, "Failed to retrieve '%s'", function_names[i]);
                return_value = MINEC_CLIENT_ERROR;
            }
        }
    }

#ifdef MINEC_CLIENT_DEBUG

    if (return_value == MINEC_CLIENT_SUCCESS) {
        VkDebugUtilsMessengerCreateInfoEXT debug_info = { 0 };
        debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        debug_info.pfnUserCallback = vulkan_debug_callback;

        if (base->func.vkCreateDebugUtilsMessengerEXT(base->instance, &debug_info, 0, &base->debug_messenger) != VK_SUCCESS)
        {
            renderer_backend_vulkan_log(client, "vkCreateDebugUtilsMessengerEXT failed");
            return_value = MINEC_CLIENT_ERROR;
        }
        else
            debug_messenger_created = true;
    }
#endif

    if (return_value == MINEC_CLIENT_SUCCESS)
    {
        if (window_vkCreateSurfaceKHR(client->window.window_handle, base->instance, base->func.vkGetInstanceProcAddr, &base->surface) != VK_SUCCESS)
        {
            renderer_backend_vulkan_log(client, "vkCreateSurfaceKHR failed");
            return_value = MINEC_CLIENT_ERROR;
        }
        else
            surface_created = true;
    }

    if (return_value == MINEC_CLIENT_SUCCESS) if (base->func.vkEnumeratePhysicalDevices(base->instance, &base->physical_device_count, 0) != VK_SUCCESS)
    {
        renderer_backend_vulkan_log(client, "vkEnumeratePhysicalDevices failed");
        return_value = MINEC_CLIENT_ERROR;
    }

    if (return_value == MINEC_CLIENT_SUCCESS)
    {
        if ((base->physical_devices = s_alloc(client->static_alloc, sizeof(VkPhysicalDevice) * base->physical_device_count)) == NULL)
            return_value = MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
        else
            device_memory = true;
    }

    if (return_value == MINEC_CLIENT_SUCCESS) if (base->func.vkEnumeratePhysicalDevices(base->instance, &base->physical_device_count, base->physical_devices) != VK_SUCCESS)
    {
        renderer_backend_vulkan_log(client, "vkEnumeratePhysicalDevices failed");
        return_value = MINEC_CLIENT_ERROR;
    }

    if (return_value == MINEC_CLIENT_SUCCESS)
    {
        if ((base->physical_device_properties = s_alloc(client->static_alloc, sizeof(VkPhysicalDeviceProperties) * base->physical_device_count)) == NULL)
            return_value = MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
        else
            device_properties_memory = true;
    }

    if (return_value == MINEC_CLIENT_SUCCESS) 
    {

        if ((base->backend_device_infos = s_alloc(client->static_alloc, sizeof(uint8_t*) * base->physical_device_count)) == NULL)
            return_value = MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
        else
        {
            for (uint32_t i = 0; i < base->physical_device_count; i++)
            {
                base->func.vkGetPhysicalDeviceProperties(base->physical_devices[i], &base->physical_device_properties[i]);

                if ((base->backend_device_infos[i] = s_alloc_string(client->static_alloc, "%s", base->physical_device_properties[i].deviceName)) == NULL)
                {
                    return_value = MINEC_CLIENT_ERROR_OUT_OF_MEMORY;

                    for (uint32_t j = 0; j < i; j++) s_free(client->static_alloc, base->backend_device_infos[j]);
                    s_free(client->static_alloc, base->backend_device_infos);

                    break;
                }
            }
        }
    }

    if (return_value == MINEC_CLIENT_SUCCESS)
    {
        *device_count = base->physical_device_count;
        *device_infos = base->backend_device_infos;
        *backend_base = base;
    }

    if (return_value != MINEC_CLIENT_SUCCESS && device_properties_memory) s_free(client->static_alloc, base->physical_device_properties);
    if (return_value != MINEC_CLIENT_SUCCESS && device_memory) s_free(client->static_alloc, base->physical_devices);
    if (return_value != MINEC_CLIENT_SUCCESS && surface_created) base->func.vkDestroySurfaceKHR(base->instance, base->surface, 0);
#ifdef MINEC_CLIENT_DEBUG
    if (return_value != MINEC_CLIENT_SUCCESS && debug_messenger_created) base->func.vkDestroyDebugUtilsMessengerEXT(base->instance, base->debug_messenger, 0);
    if (layers_memory) s_free(client->dynamic_alloc, layers);
#endif
    if (return_value != MINEC_CLIENT_SUCCESS && instance_created) base->func.vkDestroyInstance(base->instance, 0);
    if (extensions_memory) s_free(client->dynamic_alloc, extensions);
    if (return_value != MINEC_CLIENT_SUCCESS && library_loaded) dynamic_library_unload(base->func.libarary_handle);
    if (return_value != MINEC_CLIENT_SUCCESS && base_memory) s_free(client->static_alloc, base);

    return return_value;
}

void renderer_backend_vulkan_base_destroy(struct minec_client* client, void** backend_base)
{
    struct renderer_backend_vulkan_base* base = *backend_base;

    for (uint32_t i = 0; i < base->physical_device_count; i++) s_free(client->static_alloc, base->backend_device_infos[i]);
    s_free(client->static_alloc, base->backend_device_infos);

    base->func.vkDestroySurfaceKHR(base->instance, base->surface, 0);

    s_free(client->static_alloc, base->physical_device_properties);
    s_free(client->static_alloc, base->physical_devices);

#ifdef MINEC_CLIENT_DEBUG
    base->func.vkDestroyDebugUtilsMessengerEXT(base->instance, base->debug_messenger, 0);
#endif

    base->func.vkDestroyInstance(base->instance, 0);

    dynamic_library_unload(base->func.libarary_handle);
    s_free(client->static_alloc, base);

    window_deinit_context();
}