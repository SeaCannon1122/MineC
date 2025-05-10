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

uint32_t renderer_backend_vulkan_base_create(struct minec_client* client, uint32_t slot_index, uint32_t* device_count, uint8_t*** device_infos)
{
    uint32_t return_value = MINEC_CLIENT_SUCCESS;

    bool 
        window_context_initialized = false,
        backend_memory = false, 
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

    struct renderer_backend_vulkan* backend;

    if ((backend = s_alloc(client->static_alloc, sizeof(struct renderer_backend_vulkan))) == NULL)
        return_value = MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
    else 
        backend_memory = true;

    if (return_value == MINEC_CLIENT_SUCCESS)
    {
        if ((backend->func.libarary_handle = dynamic_library_load(MINEC_CLIENT_VULKAN_LIBRARY_NAME, true)) == NULL)
        {
            renderer_backend_vulkan_log(client, "Failed to load '%s'", MINEC_CLIENT_VULKAN_LIBRARY_NAME);
            return_value = MINEC_CLIENT_ERROR;
        }
        else 
            library_loaded = true;
    }

    if (return_value == MINEC_CLIENT_SUCCESS) {

        if ((backend->func.vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dynamic_library_get_function(backend->func.libarary_handle, "vkGetInstanceProcAddr")) == NULL)
        {
            renderer_backend_vulkan_log(client, "Failed to retrieve 'vkGetInstanceProcAddr'");
            return_value = MINEC_CLIENT_ERROR;
        }

        void** functions[] = {
            (void**) &backend->func.vkEnumerateInstanceExtensionProperties,
            (void**) &backend->func.vkEnumerateInstanceLayerProperties,
#ifdef MINEC_CLIENT_DEBUG
            (void**) &backend->func.vkCreateDebugUtilsMessengerEXT,
            (void**) &backend->func.vkDestroyDebugUtilsMessengerEXT,
#endif
            (void**) &backend->func.vkCreateInstance,
            (void**) &backend->func.vkDestroyInstance,
        };

        uint8_t* function_names[] = {
            "vkEnumerateInstanceExtensionProperties",
            "vkEnumerateInstanceLayerProperties",
#ifdef MINEC_CLIENT_DEBUG
            "vkCreateDebugUtilsMessengerEXT",
            "vkDestroyDebugUtilsMessengerEXT",
#endif
            "vkCreateInstance",
            "vkDestroyInstance",
        };

        for (uint32_t i = 0; i < sizeof(functions) / sizeof(functions[0]) && return_value == MINEC_CLIENT_SUCCESS; i++)
        {
            if ((*functions[i] = backend->func.vkGetInstanceProcAddr(NULL, function_names[i])) == NULL)
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

    if (return_value == MINEC_CLIENT_SUCCESS) if (backend->func.vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL) != VK_SUCCESS)
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

    if (return_value == MINEC_CLIENT_SUCCESS) if (backend->func.vkEnumerateInstanceExtensionProperties(NULL, &extension_count, extensions) != VK_SUCCESS)
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

    if (return_value == MINEC_CLIENT_SUCCESS) if (backend->func.vkEnumerateInstanceLayerProperties(&layer_count, NULL) != VK_SUCCESS)
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

    if (return_value == MINEC_CLIENT_SUCCESS) if (backend->func.vkEnumerateInstanceLayerProperties(&layer_count, layers) != VK_SUCCESS)
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


    VkApplicationInfo app_info = { 0 };
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "MineC";
    app_info.pEngineName = "MineC_Engine";

    VkInstanceCreateInfo instance_info = { 0 };
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pApplicationInfo = &app_info;
    instance_info.ppEnabledExtensionNames = (const char* const*)instance_extensions;
    instance_info.enabledExtensionCount = 4;
#ifdef MINEC_CLIENT_DEBUG
    instance_info.ppEnabledLayerNames = (const char* const*)instance_layers;
    instance_info.enabledLayerCount = 1;
#endif
    if (return_value == MINEC_CLIENT_SUCCESS)
    {
        if (backend->func.vkCreateInstance(&instance_info, 0, &backend->instance) != VK_SUCCESS)
        {
            renderer_backend_vulkan_log(client, "vkCreateInstance failed");
            return_value = MINEC_CLIENT_ERROR;
        }
        else 
            instance_created = true;
    }
    

#ifdef MINEC_CLIENT_DEBUG
    VkDebugUtilsMessengerCreateInfoEXT debug_info = { 0 };
    debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debug_info.pfnUserCallback = vulkan_debug_callback;

    if (return_value == MINEC_CLIENT_SUCCESS) {
        if (backend->func.vkCreateDebugUtilsMessengerEXT(backend->instance, &debug_info, 0, &backend->debug_messenger) != VK_SUCCESS)
        {
            renderer_backend_vulkan_log(client, "vkCreateDebugUtilsMessengerEXT failed");
            return_value = MINEC_CLIENT_ERROR;
        }
        else
            debug_messenger_created = true;
    }
#endif

    {
        void** functions[] = {
            (void**) &backend->func.vkEnumeratePhysicalDevices,
            (void**) &backend->func.vkGetPhysicalDeviceProperties,
            (void**) &backend->func.vkDestroySurfaceKHR,
            (void**) &backend->func.vkCreateDevice,
            (void**) &backend->func.vkDestroyDevice,
        };

        uint8_t* function_names[] = {
            "vkEnumeratePhysicalDevices",
            "vkGetPhysicalDeviceProperties",
            "vkDestroySurfaceKHR",
            "vkCreateDevice",
            "vkDestroyDevice"
        };

        for (uint32_t i = 0; i < sizeof(functions) / sizeof(functions[0]) && return_value == MINEC_CLIENT_SUCCESS; i++)
        {
            if ((*functions[i] = backend->func.vkGetInstanceProcAddr(backend->instance, function_names[i])) == NULL)
            {
                renderer_backend_vulkan_log(client, "Failed to retrieve '%s'", function_names[i]);
                return_value = MINEC_CLIENT_ERROR;
            }
        }
    }

    if (return_value == MINEC_CLIENT_SUCCESS)
    {
        if (window_vkCreateSurfaceKHR(client->window.window_handle, backend->instance, backend->func.vkGetInstanceProcAddr, &backend->surface) != VK_SUCCESS)
        {
            renderer_backend_vulkan_log(client, "vkCreateSurfaceKHR failed");
            return_value = MINEC_CLIENT_ERROR;
        }
        else
            surface_created = true;
    }

    if (return_value == MINEC_CLIENT_SUCCESS) if (backend->func.vkEnumeratePhysicalDevices(backend->instance, &backend->physical_device_count, 0) != VK_SUCCESS)
    {
        renderer_backend_vulkan_log(client, "vkEnumeratePhysicalDevices failed");
        return_value = MINEC_CLIENT_ERROR;
    }

    if (backend->physical_device_count == 0)
    {
        renderer_backend_vulkan_log(client, "No VkPhysicalDevice found");
        return_value = MINEC_CLIENT_ERROR;
    }

    if (return_value == MINEC_CLIENT_SUCCESS)
    {
        if ((backend->physical_devices = s_alloc(client->static_alloc, sizeof(VkPhysicalDevice) * backend->physical_device_count)) == NULL)
            return_value = MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
        else
            device_memory = true;
    }

    if (return_value == MINEC_CLIENT_SUCCESS) if (backend->func.vkEnumeratePhysicalDevices(backend->instance, &backend->physical_device_count, backend->physical_devices) != VK_SUCCESS)
    {
        renderer_backend_vulkan_log(client, "vkEnumeratePhysicalDevices failed");
        return_value = MINEC_CLIENT_ERROR;
    }

    if (return_value == MINEC_CLIENT_SUCCESS)
    {
        if ((backend->physical_device_properties = s_alloc(client->static_alloc, sizeof(VkPhysicalDeviceProperties) * backend->physical_device_count)) == NULL)
            return_value = MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
        else
            device_properties_memory = true;
    }

    if (return_value == MINEC_CLIENT_SUCCESS) 
    {

        if ((backend->backend_device_infos = s_alloc(client->static_alloc, sizeof(uint8_t*) * backend->physical_device_count)) == NULL)
            return_value = MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
        else
        {
            for (uint32_t i = 0; i < backend->physical_device_count; i++)
            {
                backend->func.vkGetPhysicalDeviceProperties(backend->physical_devices[i], &backend->physical_device_properties[i]);

                if ((backend->backend_device_infos[i] = s_alloc_string(client->static_alloc, "%s", backend->physical_device_properties[i].deviceName)) == NULL)
                {
                    return_value = MINEC_CLIENT_ERROR_OUT_OF_MEMORY;

                    for (uint32_t j = 0; j < i; j++) s_free(client->static_alloc, backend->backend_device_infos[j]);
                    s_free(client->static_alloc, backend->backend_device_infos);

                    break;
                }
            }
        }
    }

    if (return_value == MINEC_CLIENT_SUCCESS)
    {
        *device_count = backend->physical_device_count;
        *device_infos = backend->backend_device_infos;
    }

    if (return_value != MINEC_CLIENT_SUCCESS && device_properties_memory) s_free(client->static_alloc, backend->physical_device_properties);
    if (return_value != MINEC_CLIENT_SUCCESS && device_memory) s_free(client->static_alloc, backend->physical_devices);
    if (return_value != MINEC_CLIENT_SUCCESS && surface_created) backend->func.vkDestroySurfaceKHR(backend->instance, backend->surface, 0);
#ifdef MINEC_CLIENT_DEBUG
    if (return_value != MINEC_CLIENT_SUCCESS && debug_messenger_created) backend->func.vkDestroyDebugUtilsMessengerEXT(backend->instance, backend->debug_messenger, 0);
    if (layers_memory) s_free(client->dynamic_alloc, layers);
#endif
    if (return_value != MINEC_CLIENT_SUCCESS && instance_created) backend->func.vkDestroyInstance(backend->instance, 0);
    if (extensions_memory) s_free(client->dynamic_alloc, extensions);
    if (return_value != MINEC_CLIENT_SUCCESS && library_loaded) dynamic_library_unload(backend->func.libarary_handle);
    if (return_value != MINEC_CLIENT_SUCCESS && backend_memory) s_free(client->static_alloc, backend);

    if (return_value == MINEC_CLIENT_SUCCESS) client->renderer.backend[slot_index] = backend;

    return return_value;
}

uint32_t renderer_backend_vulkan_base_destroy(struct minec_client* client, uint32_t slot_index)
{
    struct renderer_backend_vulkan* backend = client->renderer.backend[slot_index];

    for (uint32_t i = 0; i < backend->physical_device_count; i++) s_free(client->static_alloc, backend->backend_device_infos[i]);
    s_free(client->static_alloc, backend->backend_device_infos);

    backend->func.vkDestroySurfaceKHR(backend->instance, backend->surface, 0);

    s_free(client->static_alloc, backend->physical_device_properties);
    s_free(client->static_alloc, backend->physical_devices);

#ifdef MINEC_CLIENT_DEBUG
    backend->func.vkDestroyDebugUtilsMessengerEXT(backend->instance, backend->debug_messenger, 0);
#endif

    backend->func.vkDestroyInstance(backend->instance, 0);

    dynamic_library_unload(backend->func.libarary_handle);
    s_free(client->static_alloc, backend);

    window_deinit_context();
}