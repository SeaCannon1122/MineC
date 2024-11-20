#include "pixel_char.h"

#include <stdio.h>
#include <malloc.h>

#define __PIXEL_CHAR_IF_BIT(ptr, pos) (((char*)ptr)[pos / 8] & (1 << (pos % 8)) )

#define __PIXEL_CHAR_WIDTH(c, font_map) (((struct pixel_font*)((font_map)[(c).masks & PIXEL_CHAR_FONT_MASK]))->char_font_entries[(c).value].width)

static void* __load_file(uint8_t* filename, uint32_t* size) {

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

uint32_t pixel_char_renderer_new(struct pixel_char_renderer* renderer, VkDevice device, VkRenderPass render_pass) {
	renderer->device = device;

	VkPipelineLayoutCreateInfo layout_info = { 0 };
	layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	VKCall(vkCreatePipelineLayout(device, &layout_info, 0, &renderer->pipe_layout));


	VkPipelineVertexInputStateCreateInfo vertex_input_stage = { 0 };
	vertex_input_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkPipelineColorBlendAttachmentState color_blend_attachment = { 0 };
	color_blend_attachment.blendEnable = VK_FALSE;
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;


	VkPipelineColorBlendStateCreateInfo color_blend_state = { 0 };
	color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_state.pAttachments = &color_blend_attachment;
	color_blend_state.attachmentCount = 1;

	VkPipelineInputAssemblyStateCreateInfo input_assembly = { 0 };
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkRect2D initial_scissor = { 0 };
	VkViewport initial_viewport = { 0 };
	initial_viewport.maxDepth = 1.0f;

	VkPipelineViewportStateCreateInfo viewport_state = { 0 };
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = &initial_scissor;
	viewport_state.viewportCount = 1;
	viewport_state.pViewports = &initial_viewport;

	VkPipelineRasterizationStateCreateInfo rasterization_state = { 0 };
	rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_state.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterization_state.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterization_state.polygonMode = VK_POLYGON_MODE_FILL;
	rasterization_state.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo multi_sample_state = { 0 };
	multi_sample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multi_sample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkShaderModule vertex_shader, fragment_shader;

	uint32_t vertex_shader_code_size;
	char* vertex_shader_code = __load_file("../../../resources/vertex_shader.spv", &vertex_shader_code_size);

	uint32_t fragment_shader_code_size;
	char* fragment_shader_code = __load_file("../../../resources/fragment_shader.spv", &fragment_shader_code_size);

	VkShaderModuleCreateInfo shader_info = { 0 };
	shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;


	shader_info.pCode = vertex_shader_code;
	shader_info.codeSize = vertex_shader_code_size;
	VKCall(vkCreateShaderModule(device, &shader_info, 0, &vertex_shader));

	shader_info.pCode = fragment_shader_code;
	shader_info.codeSize = fragment_shader_code_size;
	VKCall(vkCreateShaderModule(device, &shader_info, 0, &fragment_shader));

	free(vertex_shader_code);
	free(fragment_shader_code);

	VkPipelineShaderStageCreateInfo vertex_stage = { 0 };
	vertex_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertex_stage.pName = "main";
	vertex_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertex_stage.module = vertex_shader;

	VkPipelineShaderStageCreateInfo fragment_stage = { 0 };
	fragment_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragment_stage.pName = "main";
	fragment_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragment_stage.module = fragment_shader;

	VkPipelineShaderStageCreateInfo shader_stage[] = {
		vertex_stage,
		fragment_stage
	};

	VkDynamicState dynamic_states[] = {
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_VIEWPORT
	};

	VkPipelineDynamicStateCreateInfo dynamic_state = { 0 };
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.pDynamicStates = dynamic_states;
	dynamic_state.dynamicStateCount = 2;

	VkGraphicsPipelineCreateInfo pipe_info = { 0 };
	pipe_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipe_info.pVertexInputState = &vertex_input_stage;
	pipe_info.pColorBlendState = &color_blend_state;
	pipe_info.pStages = shader_stage;
	pipe_info.layout = renderer->pipe_layout;
	pipe_info.renderPass = render_pass;
	pipe_info.stageCount = 2;
	pipe_info.pRasterizationState = &rasterization_state;
	pipe_info.pViewportState = &viewport_state;
	pipe_info.pDynamicState = &dynamic_state;
	pipe_info.pMultisampleState = &multi_sample_state;
	pipe_info.pInputAssemblyState = &input_assembly;

	VKCall(vkCreateGraphicsPipelines(device, 0, 1, &pipe_info, 0, &renderer->pipeline));

	vkDestroyShaderModule(device, vertex_shader, 0);
	vkDestroyShaderModule(device, fragment_shader, 0);
}

struct pixel_font* load_pixel_font(char* src) {

	FILE* file = fopen(src, "rb");
	if (file == NULL) return NULL;

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	rewind(file);

	char* buffer = (char*)malloc(sizeof(struct pixel_font));
	if (buffer == NULL) {
		fclose(file);
		return NULL;
	}

	size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);

	fclose(file);

	return buffer;
}

uint32_t pixel_char_get_hover_index(const struct pixel_char* RESTRICT string, uint32_t text_size, int32_t line_spacing, int32_t x, int32_t y, int32_t alignment_x, int32_t alignment_y, int32_t max_width, uint32_t max_lines, const const void** RESTRICT font_map, int x_hover, int y_hover) {
	if (string->value == '\0') return -1;

	int lines = 1;
	int line_width = 0;
	for (int i = 0; string[i].value != '\0'; i++) {

		if (string[i].value == '\n') { lines++; line_width = 0; continue; }

		if (line_width == 0) line_width = (__PIXEL_CHAR_WIDTH(string[i], font_map) + 1) / 2 * text_size;
		else {
			char c = string[i].value;
			int new_line_width = line_width + (__PIXEL_CHAR_WIDTH(string[i], font_map) + 1) / 2 * text_size + (__PIXEL_CHAR_WIDTH(string[i], font_map) > 0 ? text_size : 0);
			if (new_line_width > max_width) { lines++; line_width = 0; i--; }
			else line_width = new_line_width;
		}

	}

	int y_pos = y - (alignment_y == ALIGNMENT_TOP ? 0 : (alignment_y == ALIGNMENT_BOTTOM ? (PIXEL_FONT_RESOULUTION / 2 + line_spacing) * text_size * lines - line_spacing * text_size : ((PIXEL_FONT_RESOULUTION / 2 + line_spacing) * text_size * lines - line_spacing * text_size) / 2));

	int y_tracer = y_pos;

	int text_i = 0;

	for (int i = 0; i < lines; i++) {
		if (string[text_i].value == '\n') continue;
		if (string[text_i].value == '\0') break;

		line_width = (__PIXEL_CHAR_WIDTH(string[text_i], font_map) + 1) / 2 * text_size;
		int last_char_line = text_i;

		for (; string[last_char_line + 1].value != '\n' && string[last_char_line + 1].value != '\0'; last_char_line++) {

			int new_line_width = line_width + (__PIXEL_CHAR_WIDTH(string[last_char_line + 1], font_map) + 1) / 2 * text_size + (__PIXEL_CHAR_WIDTH(string[last_char_line + 1], font_map) > 0 ? text_size : 0);
			if (new_line_width <= max_width) line_width = new_line_width;
			else break;

		}

		int x_pos = x - (alignment_x == ALIGNMENT_LEFT ? 0 : (alignment_x == ALIGNMENT_RIGHT ? line_width : line_width / 2));

		int x_tracer = x_pos;

		int text_i_line = text_i;

		for (; text_i_line <= last_char_line; text_i_line++) {

			if (string[text_i_line].value != '\x1f') if (x_hover >= x_tracer - (text_size + 1) / 2 && x_hover < x_tracer + (__PIXEL_CHAR_WIDTH(string[text_i_line], font_map) + 1) / 2 * text_size + text_size / 2 && y_hover >= y_tracer && y_hover < y_tracer + PIXEL_FONT_RESOULUTION * text_size / 2) return text_i_line;

			x_tracer += (__PIXEL_CHAR_WIDTH(string[text_i_line], font_map) + 1) / 2 * text_size + (__PIXEL_CHAR_WIDTH(string[text_i_line], font_map) > 0 ? text_size : 0);

		}

		text_i = text_i_line;

		if (string[text_i].value == '\0') break;

		y_tracer += (PIXEL_FONT_RESOULUTION / 2 + line_spacing) * text_size;
		if (string[text_i].value == '\n') text_i++;
	}

	return -1;
}

int pixel_char_fitting(const struct pixel_char* RESTRICT string, int text_size, const const void** RESTRICT font_map, int max_width) {

	int width = (__PIXEL_CHAR_WIDTH(string[0], font_map) + 1) / 2 * text_size;
	int amount = 1;

	for (; string[amount].value != '\n' && string[amount].value != '\0' && width <= max_width; amount++) {
		width += (__PIXEL_CHAR_WIDTH(string[amount], font_map) + 1) / 2 * text_size + (__PIXEL_CHAR_WIDTH(string[amount], font_map) > 0 ? text_size : 0);
	}

	return amount;
}



void pixel_char_print_string(struct pixel_char_renderer* RESTRICT renderer, const struct pixel_char* RESTRICT string, uint32_t text_size, int32_t line_spacing, int32_t x, int32_t y, int32_t alignment_x, int32_t alignment_y, int32_t max_width, uint32_t max_lines, uint32_t width, uint32_t height, const const void** RESTRICT font_map) {


	if (string->value == '\0') return;

	int lines = 1;

	int char_count = 0;

	int line_width = 0;
	for (int i = 0; string[i].value != '\0'; i++) {

		if (string[i].value == '\n') { lines++; line_width = 0; continue; }

		if(string[i].value != '\x1f') char_count++;

		if (line_width == 0) line_width = (__PIXEL_CHAR_WIDTH(string[i], font_map) + 1) / 2 * text_size;
		else {
			char c = string[i].value;
			int new_line_width = line_width + (__PIXEL_CHAR_WIDTH(string[i], font_map) + 1) / 2 * text_size + (__PIXEL_CHAR_WIDTH(string[i], font_map) > 0 ? text_size : 0);
			if (new_line_width > max_width) { lines++; line_width = 0; i--; }
			else line_width = new_line_width;
		}

	}

	int char_i = 0;
	float* transforms = alloca(char_count * 4 * sizeof(float));
	struct pixel_char* pixel_chars = alloca(char_count * sizeof(struct pixel_char));

	int y_pos = y - (alignment_y == ALIGNMENT_TOP ? 0 : (alignment_y == ALIGNMENT_BOTTOM ? (PIXEL_FONT_RESOULUTION / 2 + line_spacing) * text_size * lines - line_spacing * text_size : ((PIXEL_FONT_RESOULUTION / 2 + line_spacing) * text_size * lines - line_spacing * text_size) / 2));

	int y_tracer = y_pos;

	int text_i = 0;

	for (int i = 0; i < lines; i++) {
		if (string[text_i].value == '\n') continue;
		if (string[text_i].value == '\0') break;

		line_width = (__PIXEL_CHAR_WIDTH(string[text_i], font_map) + 1) / 2 * text_size;
		int last_char_line = text_i;

		for (; string[last_char_line + 1].value != '\n' && string[last_char_line + 1].value != '\0'; last_char_line++) {

			int new_line_width = line_width + (__PIXEL_CHAR_WIDTH(string[last_char_line + 1], font_map) + 1) / 2 * text_size + (__PIXEL_CHAR_WIDTH(string[last_char_line + 1], font_map) > 0 ? text_size : 0);
			if (new_line_width <= max_width) line_width = new_line_width;
			else break;

		}

		int x_pos = x - (alignment_x == ALIGNMENT_LEFT ? 0 : (alignment_x == ALIGNMENT_RIGHT ? line_width : line_width / 2));

		int x_tracer = x_pos;

		int text_i_line = text_i;

		for (; text_i_line <= last_char_line; text_i_line++) {

			if (string[text_i_line].value != '\x1f' && string[text_i_line].masks & PIXEL_CHAR_BACKGROUND_MASK) pixel_char_background_print(&string[text_i_line], text_size, x_tracer, y_tracer, screen, width, height, font_map);

			x_tracer += (__PIXEL_CHAR_WIDTH(string[text_i_line], font_map) + 1) / 2 * text_size + (__PIXEL_CHAR_WIDTH(string[text_i_line], font_map) > 0 ? text_size : 0);

		}

		x_tracer = x_pos;

		text_i_line = text_i;

		for (; text_i_line <= last_char_line; text_i_line++) {

			int char_pixel_width = (__PIXEL_CHAR_WIDTH(string[text_i_line], font_map) + 1) / 2 * text_size;

			if (string[text_i_line].value != '\x1f') {
				transforms[char_i * 4] = x_tracer;
				transforms[char_i * 4 + 1] = y_tracer;

				transforms[char_i * 4 + 2] = __PIXEL_CHAR_WIDTH(string[text_i_line], font_map);
				transforms[char_i * 4 + 3] = text_size;

				pixel_chars[char_i] = string[text_i_line];

				char_i++;

			}

			x_tracer += char_pixel_width + (__PIXEL_CHAR_WIDTH(string[text_i_line], font_map) > 0 ? text_size : 0);
			
		}
		

		text_i = text_i_line;

		if (string[text_i].value == '\0') break;

		y_tracer += (PIXEL_FONT_RESOULUTION / 2 + line_spacing) * text_size;
		if (string[text_i].value == '\n') text_i++;
	}

}
