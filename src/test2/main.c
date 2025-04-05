
#include "general/platformlib/platform/platform.h"
#include "client/application/application_window.h"

#include "general/pixelchar/src/pixelchar.h"
#include "general/rendering/rendering_memory_manager.h"

#include <malloc.h>
#include <stdio.h>

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

struct application_window window;

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
		PLATFORM_VK_SURFACE_EXTENSION,
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

	VKCall(create_vulkan_surface(instance, window.window_handle, &surface));
}

void vulkan_instance_destroy()
{
	VKCall(destroy_vulkan_surface(instance, surface));

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
		VkSurfaceFormatKHR* surface_formats = alloca(format_count * sizeof(VkSurfaceFormatKHR));
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
	framebuffer_info.width = window.width;
	framebuffer_info.height = window.height;
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
	platform_init();
	show_console_window();

	application_window_create(&window, 100, 100, 400, 400, "test2");

	vulkan_instance_create();
	vulkan_device_create();
	vulkan_device_renderpasses_create();
	vulkan_device_swapchain_and_framebuffers_create();

	struct pixelchar_renderer_vk pcr;
	pixelchar_renderer_vk_new(&pcr, device, gpu, window_render_pass, 1000, NULL, 0, NULL, 0);

	struct rendering_memory_manager rmm;
	VKCall(rendering_memory_manager_new(device, gpu, queue, queue_index, 10000000, &rmm));

	struct rendering_buffer font_buffer;

	struct pixelfont* font1 = pixelchar_load_font("resources/client/assets/fonts/default.pixelfont");
	struct pixelfont* font2 = pixelchar_load_font("resources/client/assets/fonts/default.pixelfont");

	VkBuffer_new(&rmm, sizeof(struct pixelfont) * 2, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, &font_buffer);

	VkBuffer_fill(&rmm, &font_buffer, font1, sizeof(struct pixelfont), 0);
	VkBuffer_fill(&rmm, &font_buffer, font2, sizeof(struct pixelfont), sizeof(struct pixelfont));

	pixelchar_renderer_vk_add_font(&pcr, font_buffer.buffer, 0, 0);
	pixelchar_renderer_vk_add_font(&pcr, font_buffer.buffer, sizeof(struct pixelfont), 1);

	while (application_window_handle_events(&window) == 0) {

		if (window.frame_flags & FRAME_FLAG_RESIZE)
		{
			vulkan_device_swapchain_and_framebuffers_destroy();
			vulkan_device_swapchain_and_framebuffers_create();
		}

		if (window.frame_flags & FRAME_FLAG_RENDERABLE)
		{
			char pixel_str[] = "WWLLOW!";

#define string_to_pixel_char(name, str, size, x, y, flags, r, g, b, a, r_b, g_b, b_b, a_b) struct pixelchar name[sizeof(str) - 1];\
for(int i = 0; i < sizeof(str) - 1; i++) {\
if(i == 0) name[i] = (struct pixelchar){ { r, g, b, a }, { r_b, g_b, b_b, a_b }, str[i], {x, y}, flags, size };\
else name[i] = (struct pixelchar){ { r, g, b, a }, { r_b, g_b, b_b, a_b }, str[i], {name[i-1].position[0] + (size * ((font1->bitmaps[name[i-1].value].width + 3) / 2 )), y}, flags, size  };\
}\

			string_to_pixel_char(chars, pixel_str, 20, 100, 100, PIXELCHAR_MASK_BACKGROUND | PIXELCHAR_MASK_UNDERLINE, 255, 255, 0, 127, 127, 0, 255, 255)

			chars[1].masks |= 1;
			pixelchar_renderer_vk_add_chars(&pcr, chars, sizeof(chars) / sizeof(chars[0]));

			VKCall(vkWaitForFences(device, 1, &queue_fence, VK_TRUE, UINT64_MAX));

			VKCall(vkResetFences(device, 1, &queue_fence));

			uint32_t swapchain_image_index;
			if (vkAcquireNextImageKHR(device, swapchain, 0, aquire_semaphore, 0, &swapchain_image_index) != VK_SUCCESS) {
				printf("[RENDERER BACKEND] Error aquireing next swapchain image\n");

				return 1;
			}

			VKCall(vkResetCommandBuffer(cmd, 0));

			VkCommandBufferBeginInfo begin_info = { 0 };
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			VKCall(vkBeginCommandBuffer(cmd, &begin_info));

			VkExtent2D screen_size;
			screen_size.width = window.width;
			screen_size.height = window.height;

			VkClearColorValue clearColor = {0};
			clearColor.float32[0] = 1.0f; // Red
			clearColor.float32[1] = 1.0f; // Green
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
			pixelchar_renderer_vk_cmd_render(&pcr, cmd, screen_size);

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

			VKCall(vkQueuePresentKHR(queue, &present_info));
		}

		sleep_for_ms(16);
	}

	free(font1);
	free(font2);

	vkDeviceWaitIdle(device);

	VkBuffer_destroy(&rmm, &font_buffer);

	VKCall(rendering_memory_manager_destroy(&rmm));

	pixelchar_renderer_vk_destroy(&pcr);

	vulkan_device_swapchain_and_framebuffers_destroy();
	vulkan_device_renderpasses_destroy();
	vulkan_device_destroy();
	vulkan_instance_destroy();

	application_window_destroy(&window);

	platform_exit();
}