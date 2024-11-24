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

struct rendering_buffer_local {
	VkBuffer buffer;
	VkDeviceMemory memory;
};

struct rendering_buffer_visible {
	struct rendering_buffer_local device_buffer;
	void* host_handle;
};

VkResult new_VkInstance(uint8_t* app_name, uint8_t* engine_name, VkInstance* instance, VkDebugUtilsMessengerEXT* debug_messenger);

VkResult new_VkDevice(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice* gpu, uint32_t* queue_family_index, float queue_priority, VkDevice* device);

VkResult new_VkSwapchainKHR(VkDevice device, VkPhysicalDevice gpu, VkSurfaceKHR surface, uint32_t* images_count, VkSwapchainKHR* swapchain, VkSurfaceFormatKHR* surface_format, VkImage* swapchain_images, VkImageView* swapchain_image_views);

VkResult new_VkImage(VkDevice device, VkPhysicalDevice gpu, VkQueue graphics_queue, VkCommandPool command_pool, uint8_t* file_path, struct rendering_buffer_visible* buffer, struct rendering_image* image);

VkResult new_VkBuffer(VkDevice device, VkPhysicalDevice gpu, uint32_t size, VkBufferUsageFlags buffer_usage_flags, VkMemoryPropertyFlagBits memory_flags, struct rendering_buffer_local* buffer);
