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
    uint32_t result = MINEC_CLIENT_SUCCESS;

    bool 
        window_context_initialized = false,
        base_memory = false, 
        vulkan_loaded = false,
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

    struct renderer_backend_vulkan_base* base;

    if ((base = s_alloc(client->static_alloc, sizeof(struct renderer_backend_vulkan_base))) == NULL)
        result = MINEC_CLIENT_CRITICAL_ERROR;
    else 
        base_memory = true;

    if (result == MINEC_CLIENT_SUCCESS)
    {
        if (window_vulkan_load() == false)
        {
            renderer_backend_vulkan_log(client, "Failed to load vulkan");
            result = MINEC_CLIENT_ERROR;
        }
        else 
            vulkan_loaded = true;
    }

    if (result == MINEC_CLIENT_SUCCESS) {

        base->func.vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)window_get_vkGetInstanceProcAddr();

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

        for (uint32_t i = 0; i < sizeof(functions) / sizeof(functions[0]) && result == MINEC_CLIENT_SUCCESS; i++)
        {
            if ((*functions[i] = base->func.vkGetInstanceProcAddr(NULL, function_names[i])) == NULL)
            {
                renderer_backend_vulkan_log(client, "Failed to retrieve '%s'", function_names[i]);
                result = MINEC_CLIENT_ERROR;
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

    if (result == MINEC_CLIENT_SUCCESS) if (base->func.vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL) != VK_SUCCESS)
    {
        renderer_backend_vulkan_log(client, "vkEnumerateInstanceExtensionProperties failed");
        result = MINEC_CLIENT_ERROR;
    }

    if (result == MINEC_CLIENT_SUCCESS)
    {
        if ((extensions = s_alloc(client->dynamic_alloc, sizeof(VkExtensionProperties) * extension_count + 8)) == NULL)
            result = MINEC_CLIENT_CRITICAL_ERROR;
        else 
            extensions_memory = true;
    }

    if (result == MINEC_CLIENT_SUCCESS) if (base->func.vkEnumerateInstanceExtensionProperties(NULL, &extension_count, extensions) != VK_SUCCESS)
    {
        renderer_backend_vulkan_log(client, "vkEnumerateInstanceExtensionProperties failed");
        result = MINEC_CLIENT_ERROR;
    }

    for (uint32_t i = 0; i < sizeof(instance_extensions) / sizeof(instance_extensions[0]) && result == MINEC_CLIENT_SUCCESS; i++)
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
            result = MINEC_CLIENT_ERROR;
        }
    }


#ifdef MINEC_CLIENT_DEBUG

    uint8_t* instance_layers[] = {
        "VK_LAYER_KHRONOS_validation"
    };

    uint32_t layer_count;
    VkLayerProperties* layers;

    if (result == MINEC_CLIENT_SUCCESS) if (base->func.vkEnumerateInstanceLayerProperties(&layer_count, NULL) != VK_SUCCESS)
    {
        renderer_backend_vulkan_log(client, "vkEnumerateInstanceLayerProperties failed");
        result = MINEC_CLIENT_ERROR;
    }

    if (result == MINEC_CLIENT_SUCCESS)
    {
        if ((layers = s_alloc(client->dynamic_alloc, sizeof(VkLayerProperties) * layer_count + 8)) == NULL)
            result = MINEC_CLIENT_CRITICAL_ERROR;
        else
            layers_memory = true;
    }

    if (result == MINEC_CLIENT_SUCCESS) if (base->func.vkEnumerateInstanceLayerProperties(&layer_count, layers) != VK_SUCCESS)
    {
        renderer_backend_vulkan_log(client, "vkEnumerateInstanceLayerProperties failed");
        result = MINEC_CLIENT_ERROR;
    }

    for (uint32_t i = 0; i < sizeof(instance_layers) / sizeof(instance_layers[0]) && result == MINEC_CLIENT_SUCCESS; i++)
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
            result = MINEC_CLIENT_ERROR;
        }
    }
#endif

    if (result == MINEC_CLIENT_SUCCESS)
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
            result = MINEC_CLIENT_ERROR;
        }
        else 
            instance_created = true;
    }

    {
        struct load_entry { void** load_dst; uint8_t* func_name; };
#define LOAD_FUNC_ENTRY(func_name) {(void**)&base->func.func_name, #func_name}

        void** functions[] = {
            (void**) &base->func.vkGetDeviceProcAddr,
            (void**) &base->func.vkDestroyInstance,
            (void**) &base->func.vkEnumeratePhysicalDevices,
            (void**) &base->func.vkGetPhysicalDeviceProperties,
            (void**) &base->func.vkGetPhysicalDeviceSurfaceFormatsKHR,
            (void**) &base->func.vkGetPhysicalDeviceQueueFamilyProperties,
            (void**) &base->func.vkGetPhysicalDeviceSurfaceSupportKHR,
            (void**) &base->func.vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
            (void**) &base->func.vkGetPhysicalDeviceSurfacePresentModesKHR,
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
            "vkGetPhysicalDeviceSurfacePresentModesKHR",
            "vkEnumerateDeviceExtensionProperties",
            "vkDestroySurfaceKHR",
            "vkCreateDevice",
            "vkDestroyDevice",
#ifdef MINEC_CLIENT_DEBUG
            "vkCreateDebugUtilsMessengerEXT",
            "vkDestroyDebugUtilsMessengerEXT",
#endif
        };

        for (uint32_t i = 0; i < sizeof(functions) / sizeof(functions[0]) && result == MINEC_CLIENT_SUCCESS; i++)
        {
            if ((*functions[i] = base->func.vkGetInstanceProcAddr(base->instance, function_names[i])) == NULL)
            {
                renderer_backend_vulkan_log(client, "Failed to retrieve '%s'", function_names[i]);
                result = MINEC_CLIENT_ERROR;
            }
        }
    }

#ifdef MINEC_CLIENT_DEBUG

    if (result == MINEC_CLIENT_SUCCESS) {
        VkDebugUtilsMessengerCreateInfoEXT debug_info = { 0 };
        debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        debug_info.pfnUserCallback = vulkan_debug_callback;

        if (base->func.vkCreateDebugUtilsMessengerEXT(base->instance, &debug_info, 0, &base->debug_messenger) != VK_SUCCESS)
        {
            renderer_backend_vulkan_log(client, "vkCreateDebugUtilsMessengerEXT failed");
            result = MINEC_CLIENT_ERROR;
        }
        else
            debug_messenger_created = true;
    }
#endif

    if (result == MINEC_CLIENT_SUCCESS)
    {
        if (window_vkCreateSurfaceKHR(client->window.window_handle, base->instance, &base->surface) != VK_SUCCESS)
        {
            renderer_backend_vulkan_log(client, "vkCreateSurfaceKHR failed");
            result = MINEC_CLIENT_ERROR;
        }
        else
            surface_created = true;
    }

    if (result == MINEC_CLIENT_SUCCESS) if (base->func.vkEnumeratePhysicalDevices(base->instance, &base->physical_device_count, 0) != VK_SUCCESS)
    {
        renderer_backend_vulkan_log(client, "vkEnumeratePhysicalDevices failed");
        result = MINEC_CLIENT_ERROR;
    }

    if (result == MINEC_CLIENT_SUCCESS)
    {
        if ((base->physical_devices = s_alloc(client->static_alloc, sizeof(VkPhysicalDevice) * base->physical_device_count)) == NULL)
            result = MINEC_CLIENT_CRITICAL_ERROR;
        else
            device_memory = true;
    }

    if (result == MINEC_CLIENT_SUCCESS) if (base->func.vkEnumeratePhysicalDevices(base->instance, &base->physical_device_count, base->physical_devices) != VK_SUCCESS)
    {
        renderer_backend_vulkan_log(client, "vkEnumeratePhysicalDevices failed");
        result = MINEC_CLIENT_ERROR;
    }

    VkPhysicalDeviceProperties* physical_device_properties;

    if (result == MINEC_CLIENT_SUCCESS)
    {
        if ((physical_device_properties = s_alloc(client->dynamic_alloc, sizeof(VkPhysicalDeviceProperties) * base->physical_device_count)) == NULL)
            result = MINEC_CLIENT_CRITICAL_ERROR;
        else
            device_properties_memory = true;
    }

    if (result == MINEC_CLIENT_SUCCESS) 
    {

        if ((base->backend_device_infos = s_alloc(client->static_alloc, sizeof(uint8_t*) * base->physical_device_count)) == NULL)
            result = MINEC_CLIENT_CRITICAL_ERROR;
        else
        {
            for (uint32_t i = 0; i < base->physical_device_count; i++)
            {
                base->func.vkGetPhysicalDeviceProperties(base->physical_devices[i], &physical_device_properties[i]);

                if ((base->backend_device_infos[i] = s_alloc_string(client->static_alloc, "%s", physical_device_properties[i].deviceName)) == NULL)
                {
                    result = MINEC_CLIENT_CRITICAL_ERROR;

                    for (uint32_t j = 0; j < i; j++) s_free(client->static_alloc, base->backend_device_infos[j]);
                    s_free(client->static_alloc, base->backend_device_infos);

                    break;
                }
            }
        }
    }

    if (result == MINEC_CLIENT_SUCCESS)
    {
        *device_count = base->physical_device_count;
        *device_infos = base->backend_device_infos;
        *backend_base = base;
    }

    if (device_properties_memory) s_free(client->dynamic_alloc, physical_device_properties);
    if (result != MINEC_CLIENT_SUCCESS && device_memory) s_free(client->static_alloc, base->physical_devices);
    if (result != MINEC_CLIENT_SUCCESS && surface_created) base->func.vkDestroySurfaceKHR(base->instance, base->surface, 0);
#ifdef MINEC_CLIENT_DEBUG
    if (result != MINEC_CLIENT_SUCCESS && debug_messenger_created) base->func.vkDestroyDebugUtilsMessengerEXT(base->instance, base->debug_messenger, 0);
    if (layers_memory) s_free(client->dynamic_alloc, layers);
#endif
    if (result != MINEC_CLIENT_SUCCESS && instance_created) base->func.vkDestroyInstance(base->instance, 0);
    if (extensions_memory) s_free(client->dynamic_alloc, extensions);
    if (result != MINEC_CLIENT_SUCCESS && vulkan_loaded) window_vulkan_unload();
    if (result != MINEC_CLIENT_SUCCESS && base_memory) s_free(client->static_alloc, base);

    return result;
}

void renderer_backend_vulkan_base_destroy(struct minec_client* client, void** backend_base)
{
    struct renderer_backend_vulkan_base* base = *backend_base;

    for (uint32_t i = 0; i < base->physical_device_count; i++) s_free(client->static_alloc, base->backend_device_infos[i]);
    s_free(client->static_alloc, base->backend_device_infos);

    base->func.vkDestroySurfaceKHR(base->instance, base->surface, 0);

    s_free(client->static_alloc, base->physical_devices);

#ifdef MINEC_CLIENT_DEBUG
    base->func.vkDestroyDebugUtilsMessengerEXT(base->instance, base->debug_messenger, 0);
#endif

    base->func.vkDestroyInstance(base->instance, 0);

    window_vulkan_unload();
    s_free(client->static_alloc, base);
}