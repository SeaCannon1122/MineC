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
};

struct rendering_memory_manager {
	VkDevice device;
	VkPhysicalDevice gpu;
	VkQueue graphics_queue;
	VkCommandPool command_pool;

	VkCommandBuffer cmd;

	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	void* staging_buffer_host_handle;
};

VkResult new_VkInstance(uint8_t* app_name, uint8_t* engine_name, VkInstance* instance, VkDebugUtilsMessengerEXT* debug_messenger);

VkResult new_VkDevice(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice* gpu, uint32_t* queue_family_index, float queue_priority, VkDevice* device);

VkResult new_VkSwapchainKHR(VkDevice device, VkPhysicalDevice gpu, VkSurfaceKHR surface, uint32_t* images_count, VkSwapchainKHR* swapchain, VkSurfaceFormatKHR* surface_format, VkImage* swapchain_images, VkImageView* swapchain_image_views);

VkResult rendering_memory_manager_new(VkDevice device, VkPhysicalDevice gpu, VkQueue graphics_queue, VkCommandPool command_pool, struct rendering_memory_manager* rmm);
VkResult rendering_memory_manager_destroy(struct rendering_memory_manager* rmm);

VkResult VkBuffer_new(struct rendering_memory_manager* rmm, uint32_t size, VkBufferUsageFlags usage_flags, struct rendering_buffer* buffer);
VkResult VkBuffer_fill(struct rendering_memory_manager* rmm, struct rendering_buffer* buffer, void* data, uint32_t size);
VkResult VkBuffer_destroy(struct rendering_memory_manager* rmm, struct rendering_buffer* buffer);

VkResult VkImage_new(struct rendering_memory_manager* rmm, uint8_t* file_path, struct rendering_image* image);