#include <minec_client.h>

struct load_entry { void** load_dst; uint8_t* func_name; };
#define VULKAN_FUNCTION(name) {(void**)&VULKAN_FUNC.name, #name},

static struct renderer_backend_info vulkan_info = { .name = "Vulkan", .description = "Might give better performance" };

struct renderer_backend_info* renderer_backend_vulkan_get_info(
	struct minec_client* client
)
{
	return &vulkan_info;
}

uint32_t renderer_backend_vulkan_base_create(
	struct minec_client* client,
	cwindow_context* window_context,
	cwindow* window,
	struct renderer_backend_device_infos* device_infos
)
{
	uint32_t result = MINEC_CLIENT_SUCCESS;

	bool
		vulkan_loaded = false,
		instance_created = false
	;

	if (result == MINEC_CLIENT_SUCCESS)
	{
		if (cwindow_context_graphics_vulkan_load(window_context, &VULKAN_FUNC.vkGetInstanceProcAddr) == true) vulkan_loaded = true;
		else { minec_client_log_debug_l(client, "cwindow_context_graphics_vulkan_load() failed"); result = MINEC_CLIENT_ERROR; }
	}
	if (result == MINEC_CLIENT_SUCCESS) {
		struct load_entry load_entries[] = { VULKAN_PRE_INSTANCE_FUNCTION_LIST };

		for (uint32_t i = 0; i < sizeof(load_entries) / sizeof(load_entries[0]) && result == MINEC_CLIENT_SUCCESS; i++)
		{
			if ((*load_entries[i].load_dst = (void*)VULKAN_FUNC.vkGetInstanceProcAddr(NULL, load_entries[i].func_name)) == NULL)
			{
				minec_client_log_debug_l(client, "'vkGetInstanceProcAddr(\"%s\")' failed", load_entries[i].func_name);
				result = MINEC_CLIENT_ERROR;
			}
		}
	}

	uint8_t* required_instance_extension_names[] =
	{
		CWINDOW_VK_KHR_PLATFORM_SURFACE_EXTENSION_NAME,
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};

	/*VkExtensionProperties instance_extensions[1024];
	uint32_t instance_extension_count = = ;
   

	if (result == MINEC_CLIENT_SUCCESS) if (VULKAN_FUNC.vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL) != VK_SUCCESS)
	{
		minec_client_log_debug_l(client, "vkEnumerateInstanceExtensionProperties failed");
		result = MINEC_CLIENT_ERROR;
	}*/

	if (result == MINEC_CLIENT_SUCCESS)
	{
		VULKAN.window_context = window_context;
		VULKAN.window = window;
	}
	else
	{
		if (vulkan_loaded) cwindow_context_graphics_vulkan_unload(window_context);
	}

	return result;
}

void renderer_backend_vulkan_base_destroy(
	struct minec_client* client
)
{
	cwindow_context_graphics_vulkan_unload(VULKAN.window_context);
}

uint32_t renderer_backend_vulkan_device_create(
	struct minec_client* client,
	uint32_t device_index
)
{
	return MINEC_CLIENT_SUCCESS;
}

void renderer_backend_vulkan_device_destroy(
	struct minec_client* client
)
{

}

uint32_t renderer_backend_vulkan_swapchain_create(
	struct minec_client* client,
	uint32_t width,
	uint32_t height,
	bool vsync,
	bool triple_buffering
)
{
	return MINEC_CLIENT_SUCCESS;
}

void renderer_backend_vulkan_swapchain_destroy(
	struct minec_client* client
)
{

}

uint32_t renderer_backend_vulkan_frame_start(
	struct minec_client* client
)
{
	return MINEC_CLIENT_SUCCESS;
}

uint32_t renderer_backend_vulkan_frame_submit(
	struct minec_client* client
)
{
	return MINEC_CLIENT_SUCCESS;
}