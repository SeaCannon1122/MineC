#include <vulkan/vulkan.h>

struct rendering_context {
	 
	VkInstance instance;
	VkDebugUtilsMessengerEXT debug_messenger;
	VkSurfaceKHR surface;
	VkPhysicalDevice gpu;
	VkDevice device;
	VkQueue graphics_queue;
	VkSwapchainKHR swapchain;
	VkImage sc_images[5];
	uint32_t sc_image_count;
	VkFramebuffer framebuffers[5];
	VkImageView sc_image_views[5];

	VkCommandPool command_pool;
	VkCommandBuffer cmd;

	VkSemaphore submit_semaphore;
	VkSemaphore aquire_semaphore;
	VkFence img_available_fence;

	VkExtent2D screen_size;
};