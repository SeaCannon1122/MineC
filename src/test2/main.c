#include <stdio.h>

#include <malloc.h>

#include "general/rendering/gui/pixel_char.h"
#include "general/rendering/rendering_window.h"

VkInstance instance;
VkDebugUtilsMessengerEXT debug_messenger;

VkPhysicalDevice gpu;
VkDevice device;

VkSurfaceFormatKHR surface_format;

VkQueue queue;
uint32_t queue_index;

VkCommandPool command_pool;
VkCommandBuffer cmd;

VkSemaphore submit_semaphore;
VkSemaphore aquire_semaphore;
VkFence img_available_fence;

VkRenderPass render_pass;

int main(int argc, char* argv[]) {

	platform_init();
	show_console_window();

	uint32_t major, minor, patch;
	get_vulkan_version(&major, &minor, &patch);
	printf("Running Vulkan %d.%d.%d\n", major, minor, patch);

	VKCall(new_VkInstance("testapp", "testengine", &instance, &debug_messenger));

	VKCall(new_VkDevice(instance, &gpu, &queue_index, 1.f, &device));
	vkGetDeviceQueue(device, queue_index, 0, &queue);

	VKCall(get_surface_format(instance, gpu, VK_FORMAT_B8G8R8A8_UNORM, &surface_format));	

	rendering_window_create_rendering_pass(device, surface_format, &render_pass);

	uint32_t window = window_create(10, 10, 400, 400, "window", 1);
	uint32_t window_width = window_get_width(window);
	uint32_t window_height = window_get_height(window);

	rendering_window_new(window, instance);
	rendering_window_swapchain_create(window, gpu, device, surface_format, VK_PRESENT_MODE_FIFO_KHR, render_pass);

	
	VKCall(command_pool_create(device, queue_index, &command_pool));
	VKCall(command_buffer_allocate(device, command_pool, &cmd));

	VKCall(semaphore_create(device, &aquire_semaphore));
	VKCall(semaphore_create(device, &submit_semaphore));
	VKCall(fence_create(device, &img_available_fence));

	struct rendering_memory_manager rmm;
	VKCall(rendering_memory_manager_new(device, gpu, queue, queue_index, 10000000, &rmm));
	
	struct pixel_char_renderer pcr;
	pixel_char_renderer_new(&pcr, &rmm, device, render_pass);


	struct pixel_font* special_font = load_pixel_font("../../../resources/client/assets/fonts/special.pixelfont");
	struct pixel_font* debug_font = load_pixel_font("../../../resources/client/assets/fonts/debug.pixelfont");

	pixel_char_renderer_add_font(&pcr, &rmm, special_font);
	pixel_char_renderer_add_font(&pcr, &rmm, special_font);


	char pixel_str[] = "HELLOW WORLD!";
	
	struct pixel_render_char {
		uint8_t color[4];
		uint8_t background_color[4];
		uint32_t value;
		uint16_t position[2];
		uint16_t masks;
		uint16_t size;
	};

#define string_to_pixel_char(name, str, size, x, y, flags) struct pixel_render_char name[sizeof(str) - 1];\
for(int i = 0; i < sizeof(str) - 1; i++) {\
if(i == 0) name[i] = (struct pixel_render_char){ { 220, 220, 220, 255 }, { 255, 0, 0, 255 }, str[i], {x, y}, flags, size };\
else name[i] = (struct pixel_render_char){ { 220, 220, 220, 255 }, { 255, 0, 0, 255 }, str[i], {name[i-1].position[0] + (size * ((debug_font->char_font_entries[name[i-1].value].width + 3) / 2 )), y}, flags, size  };\
}\

	string_to_pixel_char(chars, pixel_str, 20, 100, 100, PIXEL_CHAR_SHADOW_MASK | PIXEL_CHAR_BACKGROUND_MASK | PIXEL_CHAR_CURSIVE_MASK |PIXEL_CHAR_UNDERLINE_MASK)

	pixel_char_renderer_fill_chars(&pcr, &rmm, chars, sizeof(pixel_str));

#define FRAME_TIME_FRAMES_AVERAGE 128
#define FPS 60.


	double last_frame_times[FRAME_TIME_FRAMES_AVERAGE] = { 0 };
	for (int32_t i = 0; i < FRAME_TIME_FRAMES_AVERAGE; i++) last_frame_times[i] = 1000. / FPS;

	while (!get_key_state(KEY_ESCAPE)) {

		double start_time = get_time();

		struct window_event event;
		while (window_process_next_event(&event)) if(event.type == WINDOW_EVENT_DESTROY) goto close;

		uint32_t new_width = window_get_width(window);
		uint32_t new_height = window_get_height(window);

		if (new_width != 0 && new_height != 0) {

			if (window_width != new_width || window_height != new_height) rendering_window_resize(window);

			window_width = new_width;
			window_height = new_height;
			
			uint32_t img_index;

			VKCall(vkResetFences(device, 1, &img_available_fence));

			VkRenderPassBeginInfo renderpass_begin_info = { 0 };
			rendering_window_renderpass_begin_info(window, &renderpass_begin_info, render_pass, aquire_semaphore);

			VKCall(vkResetCommandBuffer(cmd, 0));

			VkCommandBufferBeginInfo begin_info = { 0 };
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			VKCall(vkBeginCommandBuffer(cmd, &begin_info));

			vkCmdBeginRenderPass(cmd, &renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

			VkExtent2D screen_size;
			screen_size.width = window_width;
			screen_size.height = window_height;
			
			pixel_char_renderer_render(&pcr, cmd, screen_size);

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


			VKCall(vkQueueSubmit(queue, 1, &submit_info, img_available_fence));

			rendering_window_present_image(window, queue, &submit_semaphore, 1);

			VKCall(vkWaitForFences(device, 1, &img_available_fence, VK_TRUE, UINT64_MAX));

		}

		double average = 0;

		for (int32_t i = FRAME_TIME_FRAMES_AVERAGE - 2; i >= 0; i--) {
			last_frame_times[i + 1] = last_frame_times[i];
			average += last_frame_times[i];
		}

		last_frame_times[0] = get_time() - start_time;

		if ((int32_t)(1000. / FPS - last_frame_times[0]) > 1) {
			sleep_for_ms((uint32_t)(1000. / FPS - last_frame_times[0]));
			last_frame_times[0] = get_time() - start_time;
		}

		average = (average + last_frame_times[0]) / FRAME_TIME_FRAMES_AVERAGE;

		printf("               \033[0G");
		printf("FPS %f\033[0G", 1000. / average);
		
	}
	

close:

	pixel_char_renderer_destroy(&pcr);
	
	VKCall(rendering_memory_manager_destroy(&rmm));

	rendering_window_destroy(window);
	window_destroy(window);

	vkDestroySemaphore(device, submit_semaphore, 0);
	vkDestroySemaphore(device, aquire_semaphore, 0);
	vkDestroyFence(device, img_available_fence, 0);
	vkDestroyRenderPass(device, render_pass, 0);
	vkFreeCommandBuffers(device, command_pool, 1, &cmd);
	vkDestroyCommandPool(device, command_pool, 0);

	vkDestroyDevice(device, 0);

	VKCall(destroy_VkInstance(instance, debug_messenger));

	platform_exit();

	return 0;
}