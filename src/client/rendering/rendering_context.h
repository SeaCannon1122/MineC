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

struct rendering_image {
	VkImage image;
	VkDeviceMemory memory;
	VkImageView view;
};

struct rendering_buffer {
	VkBuffer buffer;
	VkDeviceMemory memory;
	void* data;
};

VkResult new_VkInstance(uint8_t* app_name, uint8_t* engine_name, VkInstance* instance, VkDebugUtilsMessengerEXT* debug_messenger);

VkResult get_first_suitable_VkPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice* gpu, uint32_t* queue_index);

VkResult new_VkDevice(VkInstance instance, VkPhysicalDevice gpu, uint32_t queue_idx, float queue_priority, VkDevice* device);

VkResult get_first_suitable_VkSurfaceFormatKHR(VkPhysicalDevice gpu, VkSurfaceKHR surface, VkSurfaceFormatKHR* surface_format);

VkResult new_SwapchainKHR(VkDevice device, VkPhysicalDevice gpu, VkSurfaceKHR surface, VkSurfaceFormatKHR surface_format, VkSwapchainKHR* swapchain);

VkResult new_VkImage(VkDevice device, VkPhysicalDevice gpu, VkQueue graphics_queue, VkCommandPool command_pool, uint8_t* file_path, struct rendering_buffer* buffer, struct rendering_image* image);

VkResult new_VkBuffer(VkDevice device, VkPhysicalDevice gpu, uint32_t size, VkBufferUsageFlags buffer_usage_flags, VkMemoryPropertyFlagBits memory_flags, struct rendering_buffer* buffer);