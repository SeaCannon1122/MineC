#include <cwindow/cwindow.h>
#include <utils.h>

#include <stdio.h>
#include <math.h>
#include <mutex.h>
#include <malloc.h>

#include <pixelchar/impl/impl_vulkan.h>

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callbck(
	VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
	VkDebugUtilsMessageTypeFlagsEXT msg_flags,
	const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
	void* p_user_data
) {
	printf("--------------------------------------------------------------------------\n\nValidation Error: %s\n\n\n\n", p_callback_data->pMessage);
	return 0;
}

#define VKCall(call) \
do { \
    VkResult result = (call); \
    if (result != VK_SUCCESS) { \
        printf("Vulkan error in \n    %s \n at %s:%d: %d\n", #call, __FILE__, __LINE__, result); \
		DEBUGBREAK();\
    } \
} while(0)

#define FRAME_COUNT 3

extern cwindow_context* window_context;
extern cwindow* window;
extern uint32_t width;
extern uint32_t height;

extern PixelcharFont* default_font;
extern Pixelchar c[128];

VkInstance instance;
VkDebugUtilsMessengerEXT debug_messenger;

VkPhysicalDevice gpus[10];
VkPhysicalDevice gpu;
uint32_t gpu_queue_indices[10];

VkDevice device;

VkSurfaceKHR surface;
VkSurfaceFormatKHR gpu_surface_formats[10];
VkSurfaceCapabilitiesKHR surface_capabilities;

VkQueue queue;
uint32_t queue_index;
VkCommandPool command_pool;
VkCommandBuffer cmds[3];
VkSemaphore aquire_semaphores[3];
VkSemaphore submit_semaphores[3];
VkFence queue_fences[3];

VkRenderPass window_render_pass;

VkFramebuffer framebuffers[10];
uint32_t swapchain_image_count;
VkSwapchainKHR swapchain;
VkImage swapchain_images[10];
VkImageView swapchain_image_views[10];

VkDeviceMemory staging_memory;
VkBuffer staging_buffer;

VkDeviceMemory vertex_memory;
VkBuffer vertex_buffer;
void* staging_memory_host_handle;

PixelcharImplVulkanFactory factory;
PixelcharImplVulkanRenderer renderer;
PixelcharImplVulkanFont font;

uint32_t vulkan_width, vulkan_height;
uint32_t frame_index = 0;

#define VULKAN_FUNCTION_LIST \
	VULKAN_FUNCTION(vkGetInstanceProcAddr)\
	VULKAN_FUNCTION(vkGetDeviceProcAddr)\
	VULKAN_FUNCTION(vkCreateInstance)\
	VULKAN_FUNCTION(vkDestroyInstance)\
	VULKAN_FUNCTION(vkDestroySurfaceKHR)\
	VULKAN_FUNCTION(vkCreateSwapchainKHR)\
	VULKAN_FUNCTION(vkDestroySwapchainKHR)\
	VULKAN_FUNCTION(vkGetSwapchainImagesKHR)\
	VULKAN_FUNCTION(vkEnumeratePhysicalDevices)\
	VULKAN_FUNCTION(vkGetPhysicalDeviceProperties)\
	VULKAN_FUNCTION(vkGetPhysicalDeviceSurfaceFormatsKHR)\
	VULKAN_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties)\
	VULKAN_FUNCTION(vkGetPhysicalDeviceSurfaceSupportKHR)\
	VULKAN_FUNCTION(vkGetPhysicalDeviceSurfaceCapabilitiesKHR)\
	VULKAN_FUNCTION(vkCreateDevice)\
	VULKAN_FUNCTION(vkDestroyDevice)\
	VULKAN_FUNCTION(vkDeviceWaitIdle)\
	VULKAN_FUNCTION(vkGetDeviceQueue)\
	VULKAN_FUNCTION(vkQueueSubmit)\
	VULKAN_FUNCTION(vkQueuePresentKHR)\
	VULKAN_FUNCTION(vkCreateBuffer)\
	VULKAN_FUNCTION(vkDestroyBuffer)\
	VULKAN_FUNCTION(vkAllocateMemory)\
	VULKAN_FUNCTION(vkFreeMemory)\
	VULKAN_FUNCTION(vkBindBufferMemory)\
	VULKAN_FUNCTION(vkGetBufferMemoryRequirements)\
	VULKAN_FUNCTION(vkGetPhysicalDeviceMemoryProperties)\
	VULKAN_FUNCTION(vkMapMemory)\
	VULKAN_FUNCTION(vkUnmapMemory)\
	VULKAN_FUNCTION(vkCreateCommandPool)\
	VULKAN_FUNCTION(vkDestroyCommandPool)\
	VULKAN_FUNCTION(vkCreateFramebuffer)\
	VULKAN_FUNCTION(vkDestroyFramebuffer)\
	VULKAN_FUNCTION(vkAllocateCommandBuffers)\
	VULKAN_FUNCTION(vkFreeCommandBuffers)\
	VULKAN_FUNCTION(vkResetCommandBuffer)\
	VULKAN_FUNCTION(vkCreateSemaphore)\
	VULKAN_FUNCTION(vkDestroySemaphore)\
	VULKAN_FUNCTION(vkCreateFence)\
	VULKAN_FUNCTION(vkDestroyFence)\
	VULKAN_FUNCTION(vkResetFences)\
	VULKAN_FUNCTION(vkWaitForFences)\
	VULKAN_FUNCTION(vkCreateImageView)\
	VULKAN_FUNCTION(vkDestroyImageView)\
	VULKAN_FUNCTION(vkCreateRenderPass)\
	VULKAN_FUNCTION(vkDestroyRenderPass)\
	VULKAN_FUNCTION(vkAcquireNextImageKHR)\
	VULKAN_FUNCTION(vkBeginCommandBuffer)\
	VULKAN_FUNCTION(vkEndCommandBuffer)\
	VULKAN_FUNCTION(vkCmdBeginRenderPass)\
	VULKAN_FUNCTION(vkCmdEndRenderPass)\
	VULKAN_FUNCTION(vkCmdSetScissor)\
	VULKAN_FUNCTION(vkCmdSetViewport)\
	VULKAN_FUNCTION(vkCmdCopyBuffer)\
	VULKAN_FUNCTION(vkCmdPipelineBarrier)\

#define VULKAN_FUNCTION(name) PFN_##name _##name = NULL;
VULKAN_FUNCTION_LIST
#undef VULKAN_FUNCTION

void load_functions(uint32_t step)
{
	void* param;
	void* (*function)(void*, uint8_t * name);

	if (step == 0)
	{
		param = NULL;
		function = _vkGetInstanceProcAddr;
	}
	if (step == 1)
	{
		param = instance;
		function = _vkGetInstanceProcAddr;
	}
	if (step == 2)
	{
		param = device;
		function = _vkGetDeviceProcAddr;
	}

#define VULKAN_FUNCTION(name) if (_##name == NULL) _##name = function(param, #name);
	VULKAN_FUNCTION_LIST
#undef VULKAN_FUNCTION

}


void vulkan_instance_create()
{
	load_functions(0);

	VkApplicationInfo app_info = { 0 };
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "blocks2";
	app_info.pEngineName = "blocks2_angine";
	app_info.apiVersion = VK_API_VERSION_1_3;

	char* instance_extensions[] = {
		CWINDOW_VK_KHR_PLATFORM_SURFACE_EXTENSION_NAME,
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};

	char* layers[] = {
		"VK_LAYER_KHRONOS_validation",
	};

	VkInstanceCreateInfo instance_info = { 0 };
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pApplicationInfo = &app_info;
	instance_info.ppEnabledExtensionNames = (const char* const*)instance_extensions;
	instance_info.enabledExtensionCount = 4;
	instance_info.ppEnabledLayerNames = (const char* const*)layers;
	instance_info.enabledLayerCount = 1;

	VKCall(_vkCreateInstance(&instance_info, 0, &instance));

	load_functions(1);

	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)_vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	debug_messenger = 0;
	if (vkCreateDebugUtilsMessengerEXT) {

		VkDebugUtilsMessengerCreateInfoEXT debug_info = { 0 };
		debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		debug_info.pfnUserCallback = vulkan_debug_callbck;

		vkCreateDebugUtilsMessengerEXT(instance, &debug_info, 0, &debug_messenger);
	}

	VKCall(cwindow_vkCreateSurfaceKHR(window, instance, &surface));
}

void vulkan_instance_destroy()
{
	_vkDestroySurfaceKHR(instance, surface, 0);

	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = _vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (vkDestroyDebugUtilsMessengerEXT != 0) {
		vkDestroyDebugUtilsMessengerEXT(instance, debug_messenger, 0);
	}

	_vkDestroyInstance(instance, 0);
}

void vulkan_device_create()
{
	uint32_t gpu_count = 0;
	VKCall(_vkEnumeratePhysicalDevices(instance, &gpu_count, 0));
	VKCall(_vkEnumeratePhysicalDevices(instance, &gpu_count, gpus));

	for (uint32_t i = 0; i < gpu_count; i++) {

		VkPhysicalDeviceProperties dev_prop;
		_vkGetPhysicalDeviceProperties(gpus[i], &dev_prop);

		if (VK_API_VERSION_MAJOR(dev_prop.apiVersion) < 1 || VK_API_VERSION_MINOR(dev_prop.apiVersion) < 2) continue;

		uint32_t format_count = 0;
		VKCall(_vkGetPhysicalDeviceSurfaceFormatsKHR(gpus[i], surface, &format_count, 0));
		VkSurfaceFormatKHR* surface_formats = malloc(format_count * sizeof(VkSurfaceFormatKHR));
		VKCall(_vkGetPhysicalDeviceSurfaceFormatsKHR(gpus[i], surface, &format_count, surface_formats));

		uint32_t found_format = 0;
		for (uint32_t k = 0; k < format_count; k++) if (surface_formats[k].format == VK_FORMAT_B8G8R8A8_UNORM) {
			gpu_surface_formats[i] = surface_formats[k];
			found_format = 1;
			break;
		}
		if (found_format == 0) continue;

		uint32_t queue_family_count = 0;
		_vkGetPhysicalDeviceQueueFamilyProperties(gpus[i], &queue_family_count, 0);
		VkQueueFamilyProperties* queue_props = alloca(queue_family_count * sizeof(VkQueueFamilyProperties));
		_vkGetPhysicalDeviceQueueFamilyProperties(gpus[i], &queue_family_count, queue_props);

		for (uint32_t j = 0; j < queue_family_count; j++) {

			if ((queue_props[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queue_props[j].queueFlags & VK_QUEUE_TRANSFER_BIT) && (queue_props[j].queueFlags & VK_QUEUE_COMPUTE_BIT)) {

				VkBool32 surface_support = VK_FALSE;
				VKCall(_vkGetPhysicalDeviceSurfaceSupportKHR(gpus[i], j, surface, &surface_support));

				if (surface_support) {
					gpu_queue_indices[i] = j;

				}

			}

		}

	}

	gpu = gpus[0];

	VKCall(_vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surface_capabilities));

	float queue_priority = 1.f;

	VkDeviceQueueCreateInfo queue_info = { 0 };
	queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info.queueFamilyIndex = queue_index;
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

	VKCall(_vkCreateDevice(gpu, &device_info, 0, &device));
	_vkGetDeviceQueue(device, queue_index, 0, &queue);

	load_functions(2);

	VkCommandPoolCreateInfo pool_info = { 0 };
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = queue_index;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VKCall(_vkCreateCommandPool(device, &pool_info, 0, &command_pool));

	VkCommandBufferAllocateInfo alloc_info = { 0 };
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandBufferCount = FRAME_COUNT;
	alloc_info.commandPool = command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VKCall(_vkAllocateCommandBuffers(device, &alloc_info, cmds));

	for (uint32_t i = 0; i < FRAME_COUNT; i++)
	{
		VkSemaphoreCreateInfo sema_info = { 0 };
		sema_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VKCall(_vkCreateSemaphore(device, &sema_info, 0, &aquire_semaphores[i]));
		VKCall(_vkCreateSemaphore(device, &sema_info, 0, &submit_semaphores[i]));

		VkFenceCreateInfo fence_info = { 0 };
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VKCall(_vkCreateFence(device, &fence_info, 0, &queue_fences[i]));
	}	
}

void vulkan_device_destroy()
{
	_vkDeviceWaitIdle(device);

	_vkDestroySemaphore(device, submit_semaphores[0], 0);
	_vkDestroySemaphore(device, submit_semaphores[1], 0);
	_vkDestroySemaphore(device, submit_semaphores[2], 0);
	_vkDestroySemaphore(device, aquire_semaphores[0], 0);
	_vkDestroySemaphore(device, aquire_semaphores[1], 0);
	_vkDestroySemaphore(device, aquire_semaphores[2], 0);
	_vkDestroyFence(device, queue_fences[0], 0);
	_vkDestroyFence(device, queue_fences[1], 0);
	_vkDestroyFence(device, queue_fences[2], 0);
	_vkFreeCommandBuffers(device, command_pool, 3, cmds);
	_vkDestroyCommandPool(device, command_pool, 0);

	_vkDestroyDevice(device, 0);
}

void vulkan_device_renderpasses_create()
{
	VkAttachmentDescription color_attachment = { 0 };
	color_attachment.format = VK_FORMAT_B8G8R8A8_UNORM;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription attachments[] = {
		color_attachment
	};

	VkAttachmentReference color_attachment_reference = { 0 };
	color_attachment_reference.attachment = 0;
	color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass_description = { 0 };
	subpass_description.colorAttachmentCount = 1;
	subpass_description.pColorAttachments = &color_attachment_reference;

	VkRenderPassCreateInfo render_pass_info = { 0 };
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.pAttachments = attachments;
	render_pass_info.attachmentCount = 1;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass_description;

	VKCall(_vkCreateRenderPass(device, &render_pass_info, 0, &window_render_pass));
}

void vulkan_device_renderpasses_destroy()
{
	_vkDeviceWaitIdle(device);

	_vkDestroyRenderPass(device, window_render_pass, 0);
}

void vulkan_device_swapchain_and_framebuffers_create()
{
	VkSurfaceCapabilitiesKHR surface_capabilities = { 0 };
	VKCall(_vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surface_capabilities));

	swapchain_image_count = surface_capabilities.minImageCount +
		(surface_capabilities.minImageCount == surface_capabilities.maxImageCount ? 0 : 1);

	VkSwapchainCreateInfoKHR swapchain_info = { 0 };
	swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_info.surface = surface;
	swapchain_info.minImageCount = swapchain_image_count;
	swapchain_info.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
	swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_info.preTransform = surface_capabilities.currentTransform;
	swapchain_info.imageExtent = surface_capabilities.currentExtent;
	swapchain_info.imageArrayLayers = 1;
	swapchain_info.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

	VKCall(_vkCreateSwapchainKHR(device, &swapchain_info, 0, &swapchain));

	VKCall(_vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, swapchain_images));

	VkImageViewCreateInfo image_view_info = { 0 };
	image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_info.format = VK_FORMAT_B8G8R8A8_UNORM;
	image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	image_view_info.subresourceRange.layerCount = 1;
	image_view_info.subresourceRange.levelCount = 1;

	for (uint32_t i = 0; i < swapchain_image_count; i++) {

		image_view_info.image = swapchain_images[i];
		VKCall(_vkCreateImageView(device, &image_view_info, 0, &swapchain_image_views[i]));
	}

	VkFramebufferCreateInfo framebuffer_info = { 0 };
	framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_info.width = surface_capabilities.currentExtent.width;
	framebuffer_info.height = surface_capabilities.currentExtent.height;
	framebuffer_info.renderPass = window_render_pass;
	framebuffer_info.layers = 1;
	framebuffer_info.attachmentCount = 1;

	for (uint32_t i = 0; i < swapchain_image_count; i++) {
		framebuffer_info.pAttachments = &swapchain_image_views[i];
		VKCall(_vkCreateFramebuffer(device, &framebuffer_info, 0, &framebuffers[i]));
	}
}

void vulkan_device_swapchain_and_framebuffers_destroy()
{
	_vkDeviceWaitIdle(device);

	for (uint32_t i = 0; i < swapchain_image_count; i++) {
		_vkDestroyFramebuffer(device, framebuffers[i], 0);
		_vkDestroyImageView(device, swapchain_image_views[i], 0);
	}
	_vkDestroySwapchainKHR(device, swapchain, 0);
}

void vertex_buffer_create()
{
	VkBufferCreateInfo buffer_info = { 0 };
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = FRAME_COUNT * 128 * sizeof(Pixelchar);

	buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	_vkCreateBuffer(device, &buffer_info, 0, &vertex_buffer);

	buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	_vkCreateBuffer(device, &buffer_info, 0, &staging_buffer);

	VkMemoryRequirements vertex_requirement, staging_requirements;
	_vkGetBufferMemoryRequirements(device, vertex_buffer, &vertex_requirement);
	_vkGetBufferMemoryRequirements(device, staging_buffer, &staging_requirements);

	VkPhysicalDeviceMemoryProperties memory_properties;
	_vkGetPhysicalDeviceMemoryProperties(gpu, &memory_properties);

	VkMemoryAllocateInfo allocInfo = { 0 };
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = NULL;

	allocInfo.allocationSize = vertex_requirement.size;
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) if (
		vertex_requirement.memoryTypeBits & (1 << i) &&
		(memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	)
	{
		allocInfo.memoryTypeIndex = i;
		_vkAllocateMemory(device, &allocInfo, NULL, &vertex_memory);
		break;
	}


	allocInfo.allocationSize = staging_requirements.size;
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) if (
		staging_requirements.memoryTypeBits & (1 << i) &&
		(memory_properties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) == (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
	)
	{
		allocInfo.memoryTypeIndex = i;
		_vkAllocateMemory(device, &allocInfo, NULL, &staging_memory);
		break;
	}

	_vkBindBufferMemory(device, vertex_buffer, vertex_memory, 0);
	_vkBindBufferMemory(device, staging_buffer, staging_memory, 0);

	_vkMapMemory(device, staging_memory, 0, VK_WHOLE_SIZE, 0, &staging_memory_host_handle);
}

void vertex_buffer_destroy()
{
	_vkUnmapMemory(device, staging_memory);

	_vkDestroyBuffer(device, vertex_buffer, 0);
	_vkDestroyBuffer(device, staging_buffer, 0);

	_vkFreeMemory(device, vertex_memory, 0);
	_vkFreeMemory(device, staging_memory, 0);
}

void pixelchar_callback(uint8_t* message, void* userparam);

void pixelchar_impl_init()
{
	vulkan_width = width;
	vulkan_height = height;

	cwindow_context_graphics_vulkan_load(window_context, &_vkGetInstanceProcAddr);

	vulkan_instance_create();
	vulkan_device_create();
	vulkan_device_renderpasses_create();
	vulkan_device_swapchain_and_framebuffers_create();
	vertex_buffer_create();

	PixelcharImplVulkanFactoryCreate(instance, device, gpu, queue_index, queue, _vkGetInstanceProcAddr, &factory, pixelchar_callback, NULL);
	PixelcharImplVulkanRendererCreate(&factory, window_render_pass, 0, NULL, 0, NULL, 0, &renderer, pixelchar_callback, NULL);
	PixelcharImplVulkanFontCreate(&factory, default_font, &font, pixelchar_callback, NULL);
}

void pixelchar_impl_frame()
{

	if (width != 0 && height != 0)
	{
		if (width != vulkan_width || height != vulkan_height)
		{
			vulkan_width = width;
			vulkan_height = height;

			vulkan_device_swapchain_and_framebuffers_destroy();
			vulkan_device_swapchain_and_framebuffers_create();
		}

		VKCall(_vkWaitForFences(device, 1, &queue_fences[frame_index], VK_TRUE, UINT64_MAX));

		uint32_t swapchain_image_index;
		VkResult res = _vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, aquire_semaphores[frame_index], 0, &swapchain_image_index);
		if (res != VK_SUCCESS) {
			printf("[RENDERER BACKEND] Error aquireing next swapchain image, error %d\n", res);

			return;
		}

		VKCall(_vkResetFences(device, 1, &queue_fences[frame_index]));

		VKCall(_vkResetCommandBuffer(cmds[frame_index], 0));

		VkCommandBufferBeginInfo begin_info = { 0 };
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VKCall(_vkBeginCommandBuffer(cmds[frame_index], &begin_info));

		memcpy((uintptr_t)staging_memory_host_handle + 128 * sizeof(Pixelchar) * frame_index, c, 128 * sizeof(Pixelchar));

		VkBufferCopy copy = { 0 };
		copy.size = 128 * sizeof(Pixelchar);
		copy.srcOffset = 128 * sizeof(Pixelchar) * frame_index;
		copy.dstOffset = 128 * sizeof(Pixelchar) * frame_index;

		_vkCmdCopyBuffer(cmds[frame_index], staging_buffer, vertex_buffer, 1, &copy);

		VkBufferMemoryBarrier barrier = { 0 };
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // writes from the copy
		barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT; // reads for vertex fetch
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.buffer = vertex_buffer;
		barrier.offset = 128 * sizeof(Pixelchar) * frame_index;
		barrier.size = 128 * sizeof(Pixelchar);

		_vkCmdPipelineBarrier(
			cmds[frame_index],
			VK_PIPELINE_STAGE_TRANSFER_BIT,        // srcStage: where copy happens
			VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,    // dstStage: where vertex buffer will be read
			0,
			0, NULL,                               // memory barriers (none for global memory)
			1, &barrier,                            // buffer barriers
			0, NULL                                // image barriers
		);

		VkExtent2D screen_size;
		screen_size.width = width;
		screen_size.height = height;

		VkRect2D scissor = { 0 };
		scissor.extent = screen_size;

		VkViewport viewport = { 0 };
		viewport.width = screen_size.width;
		viewport.height = screen_size.height;
		viewport.maxDepth = 1.0f;

		_vkCmdSetViewport(cmds[frame_index], 0, 1, &viewport);
		_vkCmdSetScissor(cmds[frame_index], 0, 1, &scissor);

		VkClearColorValue clearColor = { 0 };
		clearColor.float32[0] = 0.0f; // Red
		clearColor.float32[1] = 0.0f; // Green
		clearColor.float32[2] = 0.0f; // Blue
		clearColor.float32[3] = 1.0f; // Alpha

		// Set up the clear value for the color attachment
		VkClearValue clearValues[1] = { 0 };
		clearValues[0].color = clearColor;  // Assign clear color to the first attachment

		VkRenderPassBeginInfo renderpass_begin_info = { 0 };
		renderpass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpass_begin_info.renderPass = window_render_pass;
		renderpass_begin_info.renderArea.extent = screen_size;
		renderpass_begin_info.framebuffer = framebuffers[swapchain_image_index];
		renderpass_begin_info.clearValueCount = 1;
		renderpass_begin_info.pClearValues = clearValues;

		_vkCmdBeginRenderPass(cmds[frame_index], &renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

		PixelcharImplVulkanRender(&renderer, 128, vertex_buffer, 128 * sizeof(Pixelchar) * frame_index, &font, cmds[frame_index], width, height, 4.f, 4.f, 4.f, 1.4f);

		_vkCmdEndRenderPass(cmds[frame_index]);

		VKCall(_vkEndCommandBuffer(cmds[frame_index]));

		VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo submit_info = { 0 };
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pWaitDstStageMask = &wait_stage;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &cmds[frame_index];
		submit_info.pSignalSemaphores = &submit_semaphores[frame_index];
		submit_info.pWaitSemaphores = &aquire_semaphores[frame_index];
		submit_info.signalSemaphoreCount = 1;
		submit_info.waitSemaphoreCount = 1;

		VKCall(_vkQueueSubmit(queue, 1, &submit_info, queue_fences[frame_index]));



		VkPresentInfoKHR present_info = { 0 };
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.pSwapchains = &swapchain;
		present_info.swapchainCount = 1;
		present_info.pImageIndices = &swapchain_image_index;
		present_info.pWaitSemaphores = &submit_semaphores[frame_index];
		present_info.waitSemaphoreCount = 1;

		_vkQueuePresentKHR(queue, &present_info);

		frame_index = (frame_index + 1) % FRAME_COUNT;
	}
}

void pixelchar_impl_deinit()
{
	_vkDeviceWaitIdle(device);

	PixelcharImplVulkanFactoryDestroy(&factory);
	PixelcharImplVulkanRendererDestroy(&renderer);
	PixelcharImplVulkanFontDestroy(&font);

	vertex_buffer_destroy();
	vulkan_device_swapchain_and_framebuffers_destroy();
	vulkan_device_renderpasses_destroy();
	vulkan_device_destroy();
	vulkan_instance_destroy();

	cwindow_context_graphics_vulkan_unload(window_context);
}