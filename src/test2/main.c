#include <stdio.h>

#include <malloc.h>

#include "client/rendering/gui/pixel_char.h"
#include "client/rendering/rendering_window.h"

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

	uint32_t apiVersion;
	if (vkEnumerateInstanceVersion(&apiVersion) == VK_SUCCESS) {
		printf("Running Vulkan %d.%d.%d\n", VK_API_VERSION_MAJOR(apiVersion), VK_API_VERSION_MINOR(apiVersion), VK_API_VERSION_PATCH(apiVersion));
	}

	struct rendering_window window;

	VKCall(new_VkInstance("testapp", "testengine", &instance, &debug_messenger));

	VKCall(new_VkDevice(instance, &gpu, &queue_index, 1.f, &device));
	vkGetDeviceQueue(device, queue_index, 0, &queue);

	VKCall(get_surface_format(instance, gpu, VK_FORMAT_B8G8R8A8_UNORM, &surface_format));	

	VkAttachmentDescription color_attachment = { 0 };
	color_attachment.format = surface_format.format;
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

	VKCall(vkCreateRenderPass(device, &render_pass_info, 0, &render_pass));

	rendering_window_new(&window, instance, 10, 10, 400, 400, "window");
	rendering_window_swapchain_create(&window, gpu, device, surface_format, render_pass);

	
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

	VKCall(vkCreateFence(device, &fence_info, 0, &img_available_fence));

	struct rendering_memory_manager rmm;
	VKCall(rendering_memory_manager_new(device, gpu, queue, command_pool, &rmm));
	
	struct pixel_char_renderer pcr;
	pixel_char_renderer_new(&pcr, &rmm, device, render_pass);


	struct pixel_font* default_font = load_pixel_font("../../../resources/client/assets/fonts/special.pixelfont");
	struct pixel_font* debug_font = load_pixel_font("../../../resources/client/assets/fonts/debug.pixelfont");

	pixel_char_renderer_add_font(&pcr, &rmm, debug_font);
	pixel_char_renderer_add_font(&pcr, &rmm, default_font);


	char pixel_str[] = "HELLOW WORLD!";
	

#define string_to_pixel_char(name, str, size, x, y, flags) struct pixel_render_char name[sizeof(str) - 1];\
for(int i = 0; i < sizeof(str) - 1; i++) {\
if(i == 0) name[i] = (struct pixel_render_char){ size, {x, y}, { { 0.9f, 0.9f, 0.9f, 1.f }, { 1.0, 0.0, 0.0, 1.0 }, str[i], flags } };\
else name[i] = (struct pixel_render_char){ size, {name[i-1].start_position[0] + (float)(size * ((debug_font->char_font_entries[name[i-1].pixel_char_data.value].width + 3) / 2 )), y}, { { 0.9f, 0.9f, 0.9f, 1.f }, { 1.0, 0.0, 0.0, 1.0 }, str[i], flags } };\
}\


	string_to_pixel_char(chars, pixel_str, 3, 100.f, 100.f, PIXEL_CHAR_SHADOW_MASK)

	pixel_char_renderer_fill_chars(&pcr, &rmm, chars, sizeof(pixel_str));

#define FRAME_TIME_FRAMES_AVERAGE 128
#define FPS 60.


	double last_frame_times[FRAME_TIME_FRAMES_AVERAGE] = { 0 };
	for (int32_t i = 0; i < FRAME_TIME_FRAMES_AVERAGE; i++) last_frame_times[i] = 1000. / FPS;

	int32_t render = 1;

	while (!get_key_state(KEY_ESCAPE)) {

		double start_time = get_time();

		struct window_event event;
		while (window_process_next_event(&event)) if(event.type == WINDOW_EVENT_DESTROY) goto close;

		uint32_t new_width = window_get_width(window.window);
		uint32_t new_height = window_get_height(window.window);

		if (new_width != 0 && new_height != 0) {

			if (window.width != new_width || window.height != new_height) rendering_window_resize(&window, new_width, new_height);

			render = 1;
		}
		else render = 0;

		if (render) {

			
			uint32_t img_index;

			VKCall(vkWaitForFences(device, 1, &img_available_fence, VK_TRUE, UINT64_MAX));
			VKCall(vkResetFences(device, 1, &img_available_fence));

			VKCall(vkAcquireNextImageKHR(device, window.swapchain, 0, aquire_semaphore, 0, &img_index));

			VKCall(vkResetCommandBuffer(cmd, 0));

			VkCommandBufferBeginInfo begin_info = { 0 };
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			VKCall(vkBeginCommandBuffer(cmd, &begin_info));

			VkClearValue clear_value = { 0 };
			clear_value.color = (VkClearColorValue){ 0.1f, 0.5f, 1.0f, 1 };

			VkExtent2D screen_size;
			screen_size.width = window.width;
			screen_size.height = window.height;

			VkRenderPassBeginInfo renderpass_begin_info = { 0 };
			renderpass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderpass_begin_info.renderPass = render_pass;
			renderpass_begin_info.renderArea.extent = screen_size;
			renderpass_begin_info.framebuffer = window.framebuffers[img_index];
			renderpass_begin_info.pClearValues = &clear_value;
			renderpass_begin_info.clearValueCount = 1;

			vkCmdBeginRenderPass(cmd, &renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

			
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

			VkPresentInfoKHR present_info = { 0 };
			present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			present_info.pSwapchains = &window.swapchain;
			present_info.swapchainCount = 1;
			present_info.pImageIndices = &img_index;
			present_info.pWaitSemaphores = &submit_semaphore;
			present_info.waitSemaphoreCount = 1;

			VKCall(vkQueuePresentKHR(queue, &present_info));

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

	VKCall(rendering_memory_manager_destroy(&rmm));

	platform_exit();

	return 0;
}