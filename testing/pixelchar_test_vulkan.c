#include <cwindow/cwindow.h>
#include <utils.h>

#include <stdio.h>
#include <math.h>
#include <mutex.h>
#include <malloc.h>

#include <pixelchar/renderers/renderer_vulkan.h>

#if defined(_WIN32)

#define DEBUG_BREAK() __debugbreak()

#elif defined(__linux__)
#include <signal.h>
#define DEBUG_BREAK() raise(SIGTRAP)
#endif

void* loadFile(uint8_t* src, size_t* size) {

	FILE* file = fopen(src, "rb");
	if (file == NULL) return NULL;

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	rewind(file);

	void* buffer = calloc(1, fileSize);
	if (buffer == NULL) {
		fclose(file);
		return NULL;
	}

	*size = fread(buffer, 1, fileSize, file);

	fclose(file);

	return buffer;
}

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
		DEBUG_BREAK();\
    } \
} while(0)

cwindow_context* window_context;
cwindow* window;

uint32_t width;
uint32_t height;
uint32_t position_x;
uint32_t position_y;

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




#define VULKAN_FUNCTION_LIST \
	VULKAN_FUNCTION(PFN_vkGetInstanceProcAddr, _vkGetInstanceProcAddr)\
	VULKAN_FUNCTION(PFN_vkGetDeviceProcAddr, _vkGetDeviceProcAddr)\
	VULKAN_FUNCTION(PFN_vkCreateInstance, _vkCreateInstance)\
	VULKAN_FUNCTION(PFN_vkDestroyInstance, _vkDestroyInstance)\
	VULKAN_FUNCTION(PFN_vkDestroySurfaceKHR, _vkDestroySurfaceKHR)\
	VULKAN_FUNCTION(PFN_vkCreateSwapchainKHR, _vkCreateSwapchainKHR)\
	VULKAN_FUNCTION(PFN_vkDestroySwapchainKHR, _vkDestroySwapchainKHR)\
	VULKAN_FUNCTION(PFN_vkGetSwapchainImagesKHR, _vkGetSwapchainImagesKHR)\
	VULKAN_FUNCTION(PFN_vkEnumeratePhysicalDevices, _vkEnumeratePhysicalDevices)\
	VULKAN_FUNCTION(PFN_vkGetPhysicalDeviceProperties, _vkGetPhysicalDeviceProperties)\
	VULKAN_FUNCTION(PFN_vkGetPhysicalDeviceSurfaceFormatsKHR, _vkGetPhysicalDeviceSurfaceFormatsKHR)\
	VULKAN_FUNCTION(PFN_vkGetPhysicalDeviceQueueFamilyProperties, _vkGetPhysicalDeviceQueueFamilyProperties)\
	VULKAN_FUNCTION(PFN_vkGetPhysicalDeviceSurfaceSupportKHR, _vkGetPhysicalDeviceSurfaceSupportKHR)\
	VULKAN_FUNCTION(PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR, _vkGetPhysicalDeviceSurfaceCapabilitiesKHR)\
	VULKAN_FUNCTION(PFN_vkCreateDevice, _vkCreateDevice)\
	VULKAN_FUNCTION(PFN_vkDestroyDevice, _vkDestroyDevice)\
	VULKAN_FUNCTION(PFN_vkDeviceWaitIdle, _vkDeviceWaitIdle)\
	VULKAN_FUNCTION(PFN_vkGetDeviceQueue, _vkGetDeviceQueue)\
	VULKAN_FUNCTION(PFN_vkQueueSubmit, _vkQueueSubmit)\
	VULKAN_FUNCTION(PFN_vkQueuePresentKHR, _vkQueuePresentKHR)\
	VULKAN_FUNCTION(PFN_vkCreateCommandPool, _vkCreateCommandPool)\
	VULKAN_FUNCTION(PFN_vkDestroyCommandPool, _vkDestroyCommandPool)\
	VULKAN_FUNCTION(PFN_vkCreateFramebuffer, _vkCreateFramebuffer)\
	VULKAN_FUNCTION(PFN_vkDestroyFramebuffer, _vkDestroyFramebuffer)\
	VULKAN_FUNCTION(PFN_vkAllocateCommandBuffers, _vkAllocateCommandBuffers)\
	VULKAN_FUNCTION(PFN_vkFreeCommandBuffers, _vkFreeCommandBuffers)\
	VULKAN_FUNCTION(PFN_vkResetCommandBuffer, _vkResetCommandBuffer)\
	VULKAN_FUNCTION(PFN_vkCreateSemaphore, _vkCreateSemaphore)\
	VULKAN_FUNCTION(PFN_vkDestroySemaphore, _vkDestroySemaphore)\
	VULKAN_FUNCTION(PFN_vkCreateFence, _vkCreateFence)\
	VULKAN_FUNCTION(PFN_vkDestroyFence, _vkDestroyFence)\
	VULKAN_FUNCTION(PFN_vkResetFences, _vkResetFences)\
	VULKAN_FUNCTION(PFN_vkWaitForFences, _vkWaitForFences)\
	VULKAN_FUNCTION(PFN_vkCreateImageView, _vkCreateImageView)\
	VULKAN_FUNCTION(PFN_vkDestroyImageView, _vkDestroyImageView)\
	VULKAN_FUNCTION(PFN_vkCreateRenderPass, _vkCreateRenderPass)\
	VULKAN_FUNCTION(PFN_vkDestroyRenderPass, _vkDestroyRenderPass)\
	VULKAN_FUNCTION(PFN_vkAcquireNextImageKHR, _vkAcquireNextImageKHR)\
	VULKAN_FUNCTION(PFN_vkBeginCommandBuffer, _vkBeginCommandBuffer)\
	VULKAN_FUNCTION(PFN_vkEndCommandBuffer, _vkEndCommandBuffer)\
	VULKAN_FUNCTION(PFN_vkCmdBeginRenderPass, _vkCmdBeginRenderPass)\
	VULKAN_FUNCTION(PFN_vkCmdEndRenderPass, _vkCmdEndRenderPass)\
	VULKAN_FUNCTION(PFN_vkCmdSetScissor, _vkCmdSetScissor)\
	VULKAN_FUNCTION(PFN_vkCmdSetViewport, _vkCmdSetViewport)\

#define VULKAN_FUNCTION(signature, name) signature name = NULL;
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

#define VULKAN_FUNCTION(signature, name) if (name == NULL) name = function(param, &(#name)[1]);
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
	alloc_info.commandBufferCount = 3;
	alloc_info.commandPool = command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VKCall(_vkAllocateCommandBuffers(device, &alloc_info, cmds));

	VkSemaphoreCreateInfo sema_info = { 0 };
	sema_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VKCall(_vkCreateSemaphore(device, &sema_info, 0, &aquire_semaphores[0]));
	VKCall(_vkCreateSemaphore(device, &sema_info, 0, &aquire_semaphores[1]));
	VKCall(_vkCreateSemaphore(device, &sema_info, 0, &aquire_semaphores[2]));
	VKCall(_vkCreateSemaphore(device, &sema_info, 0, &submit_semaphores[0]));
	VKCall(_vkCreateSemaphore(device, &sema_info, 0, &submit_semaphores[1]));
	VKCall(_vkCreateSemaphore(device, &sema_info, 0, &submit_semaphores[2]));

	VkFenceCreateInfo fence_info = { 0 };
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VKCall(_vkCreateFence(device, &fence_info, 0, &queue_fences[0]));
	VKCall(_vkCreateFence(device, &fence_info, 0, &queue_fences[1]));
	VKCall(_vkCreateFence(device, &fence_info, 0, &queue_fences[2]));
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

int main()
{
	window_context = cwindow_context_create("context");
	cwindow_context_graphics_vulkan_load(window_context, &_vkGetInstanceProcAddr);

	window = cwindow_create(window_context, 100, 100, 500, 300, "pixelchar vulkan test", true);

	cwindow_get_dimensions(window, &width, &height, &position_x, &position_y);

	vulkan_instance_create();
	vulkan_device_create();
	vulkan_device_renderpasses_create();
	vulkan_device_swapchain_and_framebuffers_create();

	size_t default_font_data_size;
	void* default_font_data = loadFile("../../../../client/assets/minec/fonts/default.pixelfont", &default_font_data_size);
	if (default_font_data == NULL) printf("failed to load pixelfont\n");

	size_t smooth_font_data_size;
	void* smooth_font_data = loadFile("../../../../client/assets/minec/fonts/font1.pixelfont", &smooth_font_data_size);
	if (smooth_font_data == NULL) printf("failed to load pixelfont\n");

	PixelcharFont default_font;
	PixelcharResult res = pixelcharFontCreate(default_font_data, default_font_data_size, &default_font);
	PixelcharFont smooth_font;
	res = pixelcharFontCreate(smooth_font_data, smooth_font_data_size, &smooth_font);

	free(default_font_data);
	free(smooth_font_data);

	PixelcharFont fonts[PIXELCHAR_RENDERER_MAX_FONT_COUNT] = { 0 };
	fonts[0] = default_font;
	fonts[1] = smooth_font;

	PixelcharRendererVulkanCreateInfo pcr_info =
	{
		.instance = instance,
		.device = device,
		.physicalDevice = gpu,
		.pfnvkGetInstanceProcAddr = _vkGetInstanceProcAddr,
		.queue = queue,
		.queueFamilyIndex = queue_index,
		.renderPassInfoCount = 1,
		.pRenderPassInfos = &(PixelcharRendererVulkanRenderPassInfo){
			.renderPass = window_render_pass,
			.subPassCount = 1,
			.subPasses[0] = 0
		},
		.resourceFrameCount = 3,
		.maxResourceFrameCharacterCount = 100,
		.pCustomVertexShaderSource = NULL,
		.customVertexShaderSourceSize = 0,
		.pCustomFragmentShaderSource = NULL,
		.customFragmentShaderSourceSize = 0,
	};

	PixelcharRendererVulkan pcr;
	res = PixelcharRendererVulkanCreate(&pcr_info, &pcr);
	res = PixelcharRendererVulkanUseFont(pcr, default_font, 0);
	res = PixelcharRendererVulkanUseFont(pcr, smooth_font, 1);

	Pixelchar c[30];

	uint32_t frame_index = 0;

	double time = time_get();

	bool leave = false;
	while (leave == false) {

		uint32_t new_width = width, new_height = height;

		cwindow_event* event;
		while (event = cwindow_next_event(window))
		{
			switch (event->type)
			{

			case CWINDOW_EVENT_MOVE_SIZE: {
				/*printf(
					"New window dimensions:\n  width: %d\n  height: %d\n  position x: %d\n  position y: %d\n\n",
					event->info.move_size.width,
					event->info.move_size.height,
					event->info.move_size.position_x,
					event->info.move_size.position_y
				);*/

				new_width = event->info.move_size.width;
				new_height = event->info.move_size.height;
				
				
				
			} break;

			case CWINDOW_EVENT_DESTROY: {
				leave = true;
			} break;

			}
		}

		if (
			(width != new_width || height != new_height) &&
			(new_width != 0 && new_height != 0)
			)
		{
			printf("width %d -> %d |height %d -> %d\n", width, new_width, height, new_height);
			width = new_width;
			height = new_height;
			vulkan_device_swapchain_and_framebuffers_destroy();
			vulkan_device_swapchain_and_framebuffers_create();
		}

		width = new_width;
		height = new_height;


		if (width != 0 && height != 0)
		{
			double time_now = time_get();

			uint8_t buffer[11];
			snprintf(buffer, sizeof(buffer), "%.10f", time_now - time);

			time = time_now;

			uint32_t scale = 2;

			for (uint32_t i = 0; i < 10; i++)
			{
				c[i].character = buffer[i];
				c[i].flags = PIXELCHAR_BACKGROUND_BIT | PIXELCHAR_UNDERLINE_BIT | PIXELCHAR_SHADOW_BIT;
				c[i].fontIndex = i % 2;
				c[i].scale = scale;

				c[i].position[1] = 100;

				if (i == 0) c[i].position[0] = 100;
				else c[i].position[0] = c[i - 1].position[0] + pixelcharGetCharacterRenderingWidth(&c[i - 1], fonts) + pixelcharGetCharacterRenderingSpacing(&c[i - 1], &c[i], fonts);

				c[i].color[0] = 0xdc;
				c[i].color[1] = 0xdc;
				c[i].color[2] = 0xdc;
				c[i].color[3] = 255;
				c[i].backgroundColor[0] = 255;
				c[i].backgroundColor[1] = 0;
				c[i].backgroundColor[2] = 0;
				c[i].backgroundColor[3] = 255;

			}

			VKCall(_vkWaitForFences(device, 1, &queue_fences[frame_index], VK_TRUE, UINT64_MAX));

			uint32_t swapchain_image_index;
			VkResult res = _vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, aquire_semaphores[frame_index], 0, &swapchain_image_index);
			if (res != VK_SUCCESS) {
				printf("[RENDERER BACKEND] Error aquireing next swapchain image, error %d\n", res);

				continue;
			}

			VKCall(_vkResetFences(device, 1, &queue_fences[frame_index]));

			VKCall(_vkResetCommandBuffer(cmds[frame_index], 0));

			VkCommandBufferBeginInfo begin_info = { 0 };
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			VKCall(_vkBeginCommandBuffer(cmds[frame_index], &begin_info));

			PixelcharRendererVulkanResetResourceFrame(pcr, frame_index);
			PixelcharRendererVulkanUpdateRenderingData(pcr, c, 10, frame_index, cmds[frame_index]);

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

			VkClearColorValue clearColor = {0};
			clearColor.float32[0] = 0.0f; // Red
			clearColor.float32[1] = 0.0f; // Green
			clearColor.float32[2] = 0.0f; // Blue
			clearColor.float32[3] = 1.0f; // Alpha

			// Set up the clear value for the color attachment
			VkClearValue clearValues[1] = {0};
			clearValues[0].color = clearColor;  // Assign clear color to the first attachment

			VkRenderPassBeginInfo renderpass_begin_info = { 0 };
			renderpass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderpass_begin_info.renderPass = window_render_pass;
			renderpass_begin_info.renderArea.extent = screen_size;
			renderpass_begin_info.framebuffer = framebuffers[swapchain_image_index];
			renderpass_begin_info.clearValueCount = 1;
			renderpass_begin_info.pClearValues = clearValues;

			_vkCmdBeginRenderPass(cmds[frame_index], &renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

			//pixel_chars
			
			PixelcharRendererVulkanRender(pcr, window_render_pass, 0, cmds[frame_index], width, height, 4.f, 4.f, 4.f, 1.4f);

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
		
			frame_index = (frame_index + 1 ) % 3;
		}

	}

	_vkDeviceWaitIdle(device);

	PixelcharRendererVulkanDestroy(pcr);

	pixelcharFontDestroy(default_font);
	pixelcharFontDestroy(smooth_font);

	vulkan_device_swapchain_and_framebuffers_destroy();
	vulkan_device_renderpasses_destroy();
	vulkan_device_destroy();
	vulkan_instance_destroy();

	cwindow_destroy(window);

	cwindow_context_graphics_vulkan_unload(window_context);
	cwindow_context_destroy(window_context);
}