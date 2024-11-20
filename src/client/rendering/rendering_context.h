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

VkResult new_VkInstance(uint8_t* app_name, uint8_t* engine_name, VkInstance* instance, VkDebugUtilsMessengerEXT* debug_messenger);

VkResult get_first_suitable_VkPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice* gpu, uint32_t* queue_index);

VkResult new_VkDevice(VkInstance instance, VkPhysicalDevice gpu, uint32_t queue_idx, float queue_priority, VkDevice* device);