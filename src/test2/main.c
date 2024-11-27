#include "testing.h"
#include <stdio.h>
#include <math.h>

#include "general/platformlib/platform/platform.h"
#include "client/rendering/rendering_context.h"

#include <vulkan/vulkan.h>
#include <malloc.h>

#include "test2/pixel_char.h"

VkExtent2D screen_size;

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


VkRenderPass render_pass;


static void* load_file(char* filename, int* size) {

	FILE* file = fopen(filename, "rb");
	if (file == NULL) return NULL;

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	rewind(file);

	char* buffer = (char*)malloc(fileSize);
	if (buffer == NULL) {
		fclose(file);
		return NULL;
	}

	size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);

	fclose(file);

	*size = fileSize;

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

int main(int argc, char* argv[]) {

	platform_init();
	show_console_window();

	uint32_t apiVersion;
	if (vkEnumerateInstanceVersion(&apiVersion) == VK_SUCCESS) {
		printf("Running Vulkan %d.%d.%d\n", VK_API_VERSION_MAJOR(apiVersion), VK_API_VERSION_MINOR(apiVersion), VK_API_VERSION_PATCH(apiVersion));
	}

	uint32_t window = window_create(10, 10, 400, 400, "window");

	screen_size.width = window_get_width(window);
	screen_size.height = window_get_height(window);

	VKCall(new_VkInstance("testapp", "testengine", &instance, &debug_messenger));

	VKCall(create_vulkan_surface(instance, window, &surface));

	uint32_t queue_idx;

	VKCall(new_VkDevice(instance, surface, &gpu, &queue_idx, 1.f, &device));

	vkGetDeviceQueue(device, queue_idx, 0, &graphics_queue);

	VkSurfaceCapabilitiesKHR surface_capabilities = { 0 };
	VKCall(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surface_capabilities));

	sc_image_count = surface_capabilities.minImageCount + 1;
	if (surface_capabilities.minImageCount == surface_capabilities.maxImageCount) sc_image_count = surface_capabilities.minImageCount;

	VkSurfaceFormatKHR surface_format;
	VKCall(new_VkSwapchainKHR(device, gpu, surface, &sc_image_count, &swapchain, &surface_format, sc_images, sc_image_views));

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

	VkFramebufferCreateInfo framebuffer_info = { 0 };
	framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_info.width = screen_size.width;
	framebuffer_info.height = screen_size.height;
	framebuffer_info.renderPass = render_pass;
	framebuffer_info.layers = 1;
	framebuffer_info.attachmentCount = 1;

	for (uint32_t i = 0; i < sc_image_count; i++) {
		framebuffer_info.pAttachments = &sc_image_views[i];
		VKCall(vkCreateFramebuffer(device, &framebuffer_info, 0, &framebuffers[i]));
	}


	VkCommandPoolCreateInfo pool_info = { 0 };
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = queue_idx;
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
	VKCall(rendering_memory_manager_new(device, gpu, graphics_queue, command_pool, &rmm));
	
	struct pixel_char_renderer pcr;
	pixel_char_renderer_new(&pcr, &rmm, device, render_pass);


	struct pixel_font* font= load_pixel_font("../../../resources/client/assets/fonts/debug.pixelfont");

	pixel_char_renderer_add_font(&pcr, &rmm, font);


#define letter_count 20

	

#define string_to_pixel_char(name, str, size, x, y, flags) struct pixel_render_char name[sizeof(str) - 1];\
for(int i = 0; i < sizeof(str) - 1; i++) {\
if(i == 0) name[i] = (struct pixel_render_char){ size, {x, y}, { { 1.f, 1.f, 1.f, 1.f }, { 1.0, 1.0, 1.0, 1.0 }, str[i], flags } };\
else name[i] = (struct pixel_render_char){ size, {name[i-1].start_position[0] + (float)(size * ((font->char_font_entries[name[i-1].pixel_char_data.value].width + 3) / 2 )), y}, { { 1.f, 1.f, 1.f, 1.f }, { 1.0, 1.0, 1.0, 1.0 }, str[i], flags } };\
}\

	string_to_pixel_char(chars, "!!!!!!!!!!!", 10, 100.f, 100.f, PIXEL_CHAR_UNDERLINE_MASK | PIXEL_CHAR_SHADOW_MASK)

	VkBuffer_fill(&rmm, &pcr.pixel_char_buffer, chars, sizeof(struct pixel_render_char) * letter_count);

#define FRAME_TIME_FRAMES_AVERAGE 128

#define FPS 60.
	double last_frame_times[FRAME_TIME_FRAMES_AVERAGE] = { 0 };
	for (int32_t i = 0; i < FRAME_TIME_FRAMES_AVERAGE; i++) last_frame_times[i] = 1000. / FPS;

	int32_t render = 1;

	while (!get_key_state(KEY_ESCAPE)) {

		double start_time = get_time();

		struct window_event event;
		while (window_process_next_event(&event)) if(event.type == WINDOW_EVENT_DESTROY) goto close;

		uint32_t new_width = window_get_width(window);
		uint32_t new_height = window_get_height(window);

		if (new_width != 0 && new_height != 0) {

			if (screen_size.width != new_width || screen_size.height != new_height) {

				screen_size.width = new_width;
				screen_size.height = new_height;

				vkDeviceWaitIdle(device);

				for (uint32_t i = 0; i < sc_image_count; i++) {
					vkDestroyImageView(device, sc_image_views[i], 0);
					vkDestroyFramebuffer(device, framebuffers[i], 0);
				}
				vkDestroySwapchainKHR(device, swapchain, 0);

				new_VkSwapchainKHR(device, gpu, surface, &sc_image_count, &swapchain, &surface_format, sc_images, sc_image_views);

				VkFramebufferCreateInfo framebuffer_info = { 0 };
				framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebuffer_info.width = screen_size.width;
				framebuffer_info.height = screen_size.height;
				framebuffer_info.renderPass = render_pass;
				framebuffer_info.layers = 1;
				framebuffer_info.attachmentCount = 1;

				for (uint32_t i = 0; i < sc_image_count; i++) {
					framebuffer_info.pAttachments = &sc_image_views[i];
					VKCall(vkCreateFramebuffer(device, &framebuffer_info, 0, &framebuffers[i]));
				}

			}

			render = 1;
		}
		else render = 0;

		if (render) {

			

			uint32_t img_index;

			VKCall(vkWaitForFences(device, 1, &img_available_fence, VK_TRUE, UINT64_MAX));
			VKCall(vkResetFences(device, 1, &img_available_fence));

			VKCall(vkAcquireNextImageKHR(device, swapchain, 0, aquire_semaphore, 0, &img_index));

			VKCall(vkResetCommandBuffer(cmd, 0));

			VkCommandBufferBeginInfo begin_info = { 0 };
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			VKCall(vkBeginCommandBuffer(cmd, &begin_info));

			VkClearValue clear_value = { 0 };
			clear_value.color = (VkClearColorValue){ 0.0f, 1.0f, 1.0f, 1 };

			VkRenderPassBeginInfo renderpass_begin_info = { 0 };
			renderpass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderpass_begin_info.renderPass = render_pass;
			renderpass_begin_info.renderArea.extent = screen_size;
			renderpass_begin_info.framebuffer = framebuffers[img_index];
			renderpass_begin_info.pClearValues = &clear_value;
			renderpass_begin_info.clearValueCount = 1;

			vkCmdBeginRenderPass(cmd, &renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

			VkRect2D scissor = { 0 };
			scissor.extent = screen_size;

			VkViewport viewport = { 0 };
			viewport.width = screen_size.width;
			viewport.height = screen_size.height;
			viewport.maxDepth = 1.0f;

			vkCmdSetViewport(cmd, 0, 1, &viewport);
			vkCmdSetScissor(cmd, 0, 1, &scissor);

			vkCmdBindDescriptorSets(
				cmd,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				pcr.pipe_layout,
				0,
				1,
				&pcr.descriptor_set,
				0,
				0
			);

			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pcr.pipeline);

			vkCmdPushConstants(
				cmd,
				pcr.pipe_layout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(uint32_t) * 2,
				&screen_size
			);

			for (uint32_t char_render_mode = 0; char_render_mode < 3; char_render_mode++) {
				vkCmdPushConstants(
					cmd,
					pcr.pipe_layout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					sizeof(uint32_t) * 2,
					sizeof(uint32_t),
					&char_render_mode
				);

				vkCmdDraw(cmd, letter_count * 6, 1, 0, 0);
			}


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


			VKCall(vkQueueSubmit(graphics_queue, 1, &submit_info, img_available_fence));

			VkPresentInfoKHR present_info = { 0 };
			present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			present_info.pSwapchains = &swapchain;
			present_info.swapchainCount = 1;
			present_info.pImageIndices = &img_index;
			present_info.pWaitSemaphores = &submit_semaphore;
			present_info.waitSemaphoreCount = 1;

			VKCall(vkQueuePresentKHR(graphics_queue, &present_info));

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

	window_destroy(window);

	platform_exit();

	return 0;
}