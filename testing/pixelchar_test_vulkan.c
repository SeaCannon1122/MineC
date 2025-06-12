#include <pixelchar/pixelchar.h>
#include <pixelchar/backend/backend_vulkan.h>
#include <window/window.h>

#include <stdio.h>
#include <math.h>
#include <mutex.h>
#include <malloc.h>

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

static void callback(uint32_t type, uint8_t* msg)
{
	//printf("%s %s\n", (type == PIXELCHAR_DEBUG_MESSAGE_TYPE_WARNING ? "[WARNING]" : (type == PIXELCHAR_DEBUG_MESSAGE_TYPE_ERROR ? "[ERROR]" : "[CRITICAL ERROR]")), msg);
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

void* window;
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
VkCommandBuffer cmd;
VkSemaphore aquire_semaphore;
VkSemaphore submit_semaphore;
VkFence queue_fence;

VkRenderPass window_render_pass;

VkFramebuffer framebuffers[10];
uint32_t swapchain_image_count;
VkSwapchainKHR swapchain;
VkImage swapchain_images[10];
VkImageView swapchain_image_views[10];

void vulkan_instance_create()
{
	VkApplicationInfo app_info = { 0 };
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "blocks2";
	app_info.pEngineName = "blocks2_angine";
	app_info.apiVersion = VK_API_VERSION_1_3;

	

	char* instance_extensions[] = {
		window_get_VK_KHR_PLATFORM_SURFACE_EXTENSION_NAME(),
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

	VKCall(vkCreateInstance(&instance_info, 0, &instance));

	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	debug_messenger = 0;
	if (vkCreateDebugUtilsMessengerEXT) {

		VkDebugUtilsMessengerCreateInfoEXT debug_info = { 0 };
		debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		debug_info.pfnUserCallback = vulkan_debug_callbck;

		vkCreateDebugUtilsMessengerEXT(instance, &debug_info, 0, &debug_messenger);
	}

	VKCall(window_vkCreateSurfaceKHR(window, instance, &surface));
}

void vulkan_instance_destroy()
{
	vkDestroySurfaceKHR(instance, surface, 0);

	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (vkDestroyDebugUtilsMessengerEXT != 0) {
		vkDestroyDebugUtilsMessengerEXT(instance, debug_messenger, 0);
	}

	vkDestroyInstance(instance, 0);
}

void vulkan_device_create()
{
	uint32_t gpu_count = 0;
	VKCall(vkEnumeratePhysicalDevices(instance, &gpu_count, 0));
	VKCall(vkEnumeratePhysicalDevices(instance, &gpu_count, gpus));

	for (uint32_t i = 0; i < gpu_count; i++) {

		VkPhysicalDeviceProperties dev_prop;
		vkGetPhysicalDeviceProperties(gpus[i], &dev_prop);

		if (VK_API_VERSION_MAJOR(dev_prop.apiVersion) < 1 || VK_API_VERSION_MINOR(dev_prop.apiVersion) < 2) continue;

		uint32_t format_count = 0;
		VKCall(vkGetPhysicalDeviceSurfaceFormatsKHR(gpus[i], surface, &format_count, 0));
		VkSurfaceFormatKHR* surface_formats = malloc(format_count * sizeof(VkSurfaceFormatKHR));
		VKCall(vkGetPhysicalDeviceSurfaceFormatsKHR(gpus[i], surface, &format_count, surface_formats));

		uint32_t found_format = 0;
		for (uint32_t k = 0; k < format_count; k++) if (surface_formats[k].format == VK_FORMAT_B8G8R8A8_UNORM) {
			gpu_surface_formats[i] = surface_formats[k];
			found_format = 1;
			break;
		}
		if (found_format == 0) continue;

		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(gpus[i], &queue_family_count, 0);
		VkQueueFamilyProperties* queue_props = alloca(queue_family_count * sizeof(VkQueueFamilyProperties));
		vkGetPhysicalDeviceQueueFamilyProperties(gpus[i], &queue_family_count, queue_props);

		for (uint32_t j = 0; j < queue_family_count; j++) {

			if ((queue_props[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queue_props[j].queueFlags & VK_QUEUE_TRANSFER_BIT) && (queue_props[j].queueFlags & VK_QUEUE_COMPUTE_BIT)) {

				VkBool32 surface_support = VK_FALSE;
				VKCall(vkGetPhysicalDeviceSurfaceSupportKHR(gpus[i], j, surface, &surface_support));

				if (surface_support) {
					gpu_queue_indices[i] = j;

				}

			}

		}

	}

	gpu = gpus[0];

	VKCall(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surface_capabilities));

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

	VKCall(vkCreateDevice(gpu, &device_info, 0, &device));
	vkGetDeviceQueue(device, queue_index, 0, &queue);

	VkCommandPoolCreateInfo pool_info = { 0 };
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = queue_index;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VKCall(vkCreateCommandPool(device, &pool_info, 0, &command_pool));

	VkCommandBufferAllocateInfo alloc_info = { 0 };
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandBufferCount = 1;
	alloc_info.commandPool = command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VKCall(vkAllocateCommandBuffers(device, &alloc_info, &cmd));

	VkSemaphoreCreateInfo sema_info = { 0 };
	sema_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VKCall(vkCreateSemaphore(device, &sema_info, 0, &aquire_semaphore));
	VKCall(vkCreateSemaphore(device, &sema_info, 0, &submit_semaphore));

	VkFenceCreateInfo fence_info = { 0 };
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VKCall(vkCreateFence(device, &fence_info, 0, &queue_fence));
}

void vulkan_device_destroy()
{
	vkDeviceWaitIdle(device);

	vkDestroySemaphore(device, submit_semaphore, 0);
	vkDestroySemaphore(device, aquire_semaphore, 0);
	vkDestroyFence(device, queue_fence, 0);
	vkFreeCommandBuffers(device, command_pool, 1, &cmd);
	vkDestroyCommandPool(device, command_pool, 0);

	vkDestroyDevice(device, 0);
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

	VKCall(vkCreateRenderPass(device, &render_pass_info, 0, &window_render_pass));
}

void vulkan_device_renderpasses_destroy()
{
	vkDeviceWaitIdle(device);

	vkDestroyRenderPass(device, window_render_pass, 0);
}

void vulkan_device_swapchain_and_framebuffers_create()
{
	VkSurfaceCapabilitiesKHR surface_capabilities = { 0 };
	VKCall(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surface_capabilities));

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
	swapchain_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;

	VKCall(vkCreateSwapchainKHR(device, &swapchain_info, 0, &swapchain));

	VKCall(vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, swapchain_images));

	VkImageViewCreateInfo image_view_info = { 0 };
	image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_info.format = VK_FORMAT_B8G8R8A8_UNORM;
	image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	image_view_info.subresourceRange.layerCount = 1;
	image_view_info.subresourceRange.levelCount = 1;

	for (uint32_t i = 0; i < swapchain_image_count; i++) {

		image_view_info.image = swapchain_images[i];
		VKCall(vkCreateImageView(device, &image_view_info, 0, &swapchain_image_views[i]));
	}

	VkFramebufferCreateInfo framebuffer_info = { 0 };
	framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_info.width = width;
	framebuffer_info.height = height;
	framebuffer_info.renderPass = window_render_pass;
	framebuffer_info.layers = 1;
	framebuffer_info.attachmentCount = 1;

	for (uint32_t i = 0; i < swapchain_image_count; i++) {
		framebuffer_info.pAttachments = &swapchain_image_views[i];
		VKCall(vkCreateFramebuffer(device, &framebuffer_info, 0, &framebuffers[i]));
	}
}

void vulkan_device_swapchain_and_framebuffers_destroy()
{
	vkDeviceWaitIdle(device);

	for (uint32_t i = 0; i < swapchain_image_count; i++) {
		vkDestroyFramebuffer(device, framebuffers[i], 0);
		vkDestroyImageView(device, swapchain_image_views[i], 0);
	}
	vkDestroySwapchainKHR(device, swapchain, 0);
}

int main()
{
	window_init_context(NULL);
	window_vulkan_load();

	window = window_create(100, 100, 500, 300, "pixelchar vulkan test", true);

	window_get_dimensions(window, &width, &height, &position_x, &position_y);

	vulkan_instance_create();
	vulkan_device_create();
	vulkan_device_renderpasses_create();
	vulkan_device_swapchain_and_framebuffers_create();

	size_t default_font_data_size;
	void* default_font_data = loadFile("runtime_files/assets/MineCdefault/fonts/default.pixelfont", &default_font_data_size);
	if (default_font_data == NULL) printf("failed to load pixelfont\n");

	size_t smooth_font_data_size;
	void* smooth_font_data = loadFile("runtime_files/assets/MineCdefault/fonts/smooth.pixelfont", &smooth_font_data_size);
	if (smooth_font_data == NULL) printf("failed to load pixelfont\n");

	PixelcharFont default_font;
	PixelcharResult res = pixelcharFontCreate(default_font_data, default_font_data_size, &default_font);
	PixelcharFont smooth_font;
	res = pixelcharFontCreate(smooth_font_data, smooth_font_data_size, &smooth_font);

	free(default_font_data);
	free(smooth_font_data);

	PixelcharRenderer pcr;
	res = pixelcharRendererCreate(100, &pcr);
	res = pixelcharRendererBackendVulkanInitialize(pcr, 0, 3, device, gpu, queue, queue_index, window_render_pass, 0, vkGetDeviceProcAddr, 0, 0, 0, 0);
	res = pixelcharRendererBindFont(pcr, default_font, 0);
	res = pixelcharRendererBindFont(pcr, smooth_font, 1);

	pixelcharFontDestroy(default_font);
	pixelcharFontDestroy(smooth_font);

	uint8_t str[] = { 'H', 'e', 'l', 'l', 'o', ' ', 'W', 'O', 'R', 'L', 'D', '!', 128, 129, 0 };
	uint32_t str_len = sizeof(str) - 1;

	Pixelchar c[100];

	uint32_t scale = 2;

	for (uint32_t i = 0; i < str_len; i++)
	{
		c[i].character = str[i];
		c[i].flags = PIXELCHAR_BACKGROUND_BIT | PIXELCHAR_UNDERLINE_BIT | PIXELCHAR_SHADOW_BIT;
		c[i].fontIndex = i % 2;
		c[i].scale = scale;

		c[i].position[1] = 100;
		
		if (i == 0) c[i].position[0] = 100;
		else c[i].position[0] = c[i - 1].position[0] + pixelcharGetCharacterRenderingWidth(pcr, &c[i - 1]) + pixelcharGetCharacterRenderingSpacing(pcr, &c[i - 1], &c[i]);

		c[i].color[0] = 0xdc;
		c[i].color[1] = 0xdc;
		c[i].color[2] = 0xdc;
		c[i].color[3] = 255;
		c[i].backgroundColor[0] = 255;
		c[i].backgroundColor[1] = 0;
		c[i].backgroundColor[2] = 0;
		c[i].backgroundColor[3] = 255;
		
	}

	bool leave = false;
	while (leave == false) {

		struct window_event* event;
		while (event = window_next_event(window))
		{
			switch (event->type)
			{

			case WINDOW_EVENT_MOVE_SIZE: {
				printf(
					"New window dimensions:\n  width: %d\n  height: %d\n  position x: %d\n  position y: %d\n\n",
					event->info.move_size.width,
					event->info.move_size.height,
					event->info.move_size.position_x,
					event->info.move_size.position_y
				);

				width = event->info.move_size.width;
				height = event->info.move_size.height;
				if (width != 0 && height != 0) {
					vulkan_device_swapchain_and_framebuffers_destroy();
					vulkan_device_swapchain_and_framebuffers_create();
				}
				
				
			} break;

			case WINDOW_EVENT_DESTROY: {
				leave = true;
			} break;

			}
		}


		if (width != 0 && height != 0)
		{
			pixelcharRendererEnqueCharacters(pcr, c, str_len);

			VKCall(vkWaitForFences(device, 1, &queue_fence, VK_TRUE, UINT64_MAX));

			uint32_t swapchain_image_index;
			VkResult res = vkAcquireNextImageKHR(device, swapchain, 0, aquire_semaphore, 0, &swapchain_image_index);
			if (res != VK_SUCCESS) {
				printf("[RENDERER BACKEND] Error aquireing next swapchain image, error %d\n", res);

				continue;
			}

			VKCall(vkResetFences(device, 1, &queue_fence));

			VKCall(vkResetCommandBuffer(cmd, 0));

			VkCommandBufferBeginInfo begin_info = { 0 };
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			VKCall(vkBeginCommandBuffer(cmd, &begin_info));

			pixelcharRendererBackendVulkanUpdateRenderingData(pcr, 0, 0, cmd);

			VkExtent2D screen_size;
			screen_size.width = width;
			screen_size.height = height;

			VkRect2D scissor = { 0 };
			scissor.extent = screen_size;

			VkViewport viewport = { 0 };
			viewport.width = screen_size.width;
			viewport.height = screen_size.height;
			viewport.maxDepth = 1.0f;

			vkCmdSetViewport(cmd, 0, 1, &viewport);
			vkCmdSetScissor(cmd, 0, 1, &scissor);

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

			vkCmdBeginRenderPass(cmd, &renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

			//pixel_chars
			
			pixelcharRendererBackendVulkanRender(pcr, 0, cmd, width, height, 4.f, 4.f, 4.f, 1.4f);

			vkCmdEndRenderPass(cmd);

			VKCall(vkEndCommandBuffer(cmd));

			VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

			VkSubmitInfo submit_info = { 0 };
			submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.pWaitDstStageMask = &wait_stage;
			submit_info.commandBufferCount = 1;
			submit_info.pCommandBuffers = &cmd;
			submit_info.pSignalSemaphores = &submit_semaphore;
			submit_info.pWaitSemaphores = &aquire_semaphore;
			submit_info.signalSemaphoreCount = 1;
			submit_info.waitSemaphoreCount = 1;

			VKCall(vkQueueSubmit(queue, 1, &submit_info, queue_fence));



			VkPresentInfoKHR present_info = { 0 };
			present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			present_info.pSwapchains = &swapchain;
			present_info.swapchainCount = 1;
			present_info.pImageIndices = &swapchain_image_index;
			present_info.pWaitSemaphores = &submit_semaphore;
			present_info.waitSemaphoreCount = 1;

			vkQueuePresentKHR(queue, &present_info);
		}

	}

	vkDeviceWaitIdle(device);

	pixelcharRendererBackendVulkanDeinitialize(pcr, 0);
	pixelcharRendererDestroy(pcr);


	vulkan_device_swapchain_and_framebuffers_destroy();
	vulkan_device_renderpasses_destroy();
	vulkan_device_destroy();
	vulkan_instance_destroy();

	window_destroy(window);

	window_vulkan_unload();
	window_deinit_context();
}