#include "pixel_char.h"

#include "GL/glew.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include <general/platformlib/opengl_rendering.h>

#include "shader.h"

GLfloat vertexData[] = {

	0.0f, 0.0f,
	1.0f, 0.0f,
	0.0f, 1.0f,

	0.0f, 1.0f,
	1.0f, 1.0f,
	1.0f, 0.0f
};

GLuint VAO;

GLuint VBO, transform_buffer, pixel_char_buffer;

GLuint shader_program;

GLint width_uniform_location, height_uniform_location;

#define __PIXEL_CHAR_IF_BIT(ptr, pos) (((char*)ptr)[pos / 8] & (1 << (pos % 8)) )

#define __PIXEL_CHAR_WIDTH(c, font_map) (((struct pixel_font*)((font_map)[(c).masks & PIXEL_CHAR_FONT_MASK]))->char_font_entries[(c).value].width)

char* __read_text_file(char* filename) {

	FILE* file = fopen(filename, "rb");
	if (file == NULL) return NULL;

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	rewind(file);

	char* buffer_raw = (char*)malloc(fileSize);
	if (buffer_raw == NULL) {
		fclose(file);
		return NULL;
	}

	size_t bytesRead = fread(buffer_raw, sizeof(char), fileSize, file);

	int carriage_return_count = 0;

	for (int i = 0; i < fileSize; i++) if (buffer_raw[i] == '\r') carriage_return_count++;

	char* buffer = (char*)malloc(fileSize + 1 - carriage_return_count);

	int i = 0;

	for (int raw_i = 0; raw_i < fileSize; raw_i++) {
		if (buffer_raw[raw_i] != '\r') {
			buffer[i] = buffer_raw[raw_i];
			i++;
		}
	}

	buffer[fileSize - carriage_return_count] = '\0';
	free(buffer_raw);

	fclose(file);

	return buffer;
}

void pixel_char_init() {
	char* vertex_source = __read_text_file("../../../resources/font_vertex_shader.glsl");
	char* geometry_source = __read_text_file("../../../resources/font_geometry_shader.glsl");
	char* fragment_source = __read_text_file("../../../resources/font_fragment_shader.glsl");

	shader_program = shader_create_program(2, vertex_source, GL_VERTEX_SHADER, fragment_source, GL_FRAGMENT_SHADER);

	GLCall(glGenVertexArrays(1, &VAO));
	GLCall(glGenBuffers(1, &VBO));
	GLCall(glGenBuffers(1, &transform_buffer));
	GLCall(glGenBuffers(1, &pixel_char_buffer));

	GLCall(glBindVertexArray(VAO));

	GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
	GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW));
	GLCall(glEnableVertexAttribArray(0));
	GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0));

	GLCall(glBindBuffer(GL_ARRAY_BUFFER, transform_buffer));
	GLCall(glBufferData(GL_ARRAY_BUFFER, 100 * sizeof(float) * 4, NULL, GL_DYNAMIC_DRAW));
	GLCall(glEnableVertexAttribArray(2));
	GLCall(glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0));
	GLCall(glVertexAttribDivisor(2, 1));

	GLCall(glBindBuffer(GL_ARRAY_BUFFER, pixel_char_buffer));
	GLCall(glBufferData(GL_ARRAY_BUFFER, 100 * sizeof(unsigned int) * 4, NULL, GL_DYNAMIC_DRAW));
	GLCall(glEnableVertexAttribArray(1));
	GLCall(glVertexAttribIPointer(1, 4, GL_INT, sizeof(unsigned int) * 4, (void*)0));
	GLCall(glVertexAttribDivisor(1, 1));

	GLCall(glUseProgram(shader_program));

	width_uniform_location = glGetUniformLocation(shader_program, "screen_width");
	height_uniform_location = glGetUniformLocation(shader_program, "screen_height");

	GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
	GLCall(glBindVertexArray(0));

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


void pixel_char_background_print(const struct pixel_char* RESTRICT c, int text_size, int x, int y, unsigned int* RESTRICT screen, int width, int height, const const void** RESTRICT font_map) {

	int shaddow_offset = (c->masks & PIXEL_CHAR_CURSIVE_MASK ? (text_size + 1) / 2 : text_size);

	for (int j = 0; j < PIXEL_FONT_RESOULUTION * text_size / 2; j++) {

		int cursive_offset = (c->masks & PIXEL_CHAR_CURSIVE_MASK ? PIXEL_FONT_RESOULUTION * text_size / 4 - j / 2 - 1 : 0);

		for (int i = -(text_size + 1) / 2; i < (__PIXEL_CHAR_WIDTH(c[0], font_map) * text_size + 1) / 2 + text_size / 2; i++) {

			if (i + cursive_offset + x >= 0 && j + y >= 0 && i + cursive_offset + x < width && j + y < height) screen[(i + cursive_offset + x) + width * (j + y)] = c->background_color;

		}
	}
}

void pixel_char_print(const struct pixel_char* RESTRICT c, int text_size, int x, int y, unsigned int* RESTRICT screen, int width, int height, const const void** RESTRICT font_map) {

	int shaddow_offset = (c->masks & PIXEL_CHAR_CURSIVE_MASK ? (text_size + 1) / 2 : text_size);

	for (int j = 0; j < PIXEL_FONT_RESOULUTION * text_size / 2; j++) {

		int cursive_offset = (c->masks & PIXEL_CHAR_CURSIVE_MASK ? PIXEL_FONT_RESOULUTION * text_size / 4 - j / 2 - 1 : 0);
		int shaddow_offset = (c->masks & PIXEL_CHAR_CURSIVE_MASK ? (text_size + 1) / 2 : text_size);

		for (int i = 0; i < (__PIXEL_CHAR_WIDTH(c[0], font_map) * text_size + 1) / 2; i++) {

			int x_bit_pos = i - text_size * PIXEL_FONT_RESOULUTION / 4;
			int y_bit_pos = j - text_size * PIXEL_FONT_RESOULUTION / 4;

			int bit_pos = ((x_bit_pos < 0 ? x_bit_pos + 1 : x_bit_pos) * 2 / text_size + PIXEL_FONT_RESOULUTION / 2) + PIXEL_FONT_RESOULUTION * ((y_bit_pos < 0 ? y_bit_pos + 1 : y_bit_pos) * 2 / text_size + PIXEL_FONT_RESOULUTION / 2);

			if (x_bit_pos < 0) bit_pos -= 1;
			if (y_bit_pos < 0) bit_pos -= PIXEL_FONT_RESOULUTION;

			if (__PIXEL_CHAR_IF_BIT(((struct pixel_font*)(font_map[c[0].masks & PIXEL_CHAR_FONT_MASK]))->char_font_entries[c[0].value].layout, bit_pos)) {
				if (i + cursive_offset + x >= 0 && j + y >= 0 && i + cursive_offset + x < width && j + y < height) screen[(i + cursive_offset + x) + width * (j + y)] = c->color;
				if (c->masks & PIXEL_CHAR_SHADOW_MASK && i + cursive_offset + x + shaddow_offset >= 0 && j + y + text_size >= 0 && i + cursive_offset + x + shaddow_offset < width && j + y + text_size < height)
					screen[(i + cursive_offset + x + shaddow_offset) + width * (j + y + text_size)] =
					(c->color & 0xff) / PIXEL_FONT_SHADOW_DIVISOR +
					((((c->color >> 8) & 0xff) / PIXEL_FONT_SHADOW_DIVISOR) << 8) +
					((((c->color >> 16) & 0xff) / PIXEL_FONT_SHADOW_DIVISOR) << 16) +
					((((c->color >> 24) & 0xff) / PIXEL_FONT_SHADOW_DIVISOR) << 24);
			}

		}
	}

	if (c->masks & PIXEL_CHAR_UNDERLINE_MASK) {
		for (int j = PIXEL_FONT_RESOULUTION * text_size / 2; j < (PIXEL_FONT_RESOULUTION + 2) * text_size / 2; j++) {

			int cursive_offset = (c->masks & PIXEL_CHAR_CURSIVE_MASK ? PIXEL_FONT_RESOULUTION * text_size / 4 - j / 2 - 1 : 0);

			for (int i = -(text_size + 1) / 2; i < (__PIXEL_CHAR_WIDTH(c[0], font_map) * text_size + 1) / 2 + text_size / 2; i++) {

				if (i + cursive_offset + x >= 0 && j + y >= 0 && i + cursive_offset + x < width && j + y < height) screen[(i + cursive_offset + x) + width * (j + y)] = c->color;
				if (c->masks & PIXEL_CHAR_SHADOW_MASK && i + cursive_offset + x + shaddow_offset >= 0 && j + y + text_size >= 0 && i + cursive_offset + x + shaddow_offset < width && j + y + text_size < height)
					screen[(i + cursive_offset + x + shaddow_offset) + width * (j + y + text_size)] =
					(c->color & 0xff) / PIXEL_FONT_SHADOW_DIVISOR +
					((((c->color >> 8) & 0xff) / PIXEL_FONT_SHADOW_DIVISOR) << 8) +
					((((c->color >> 16) & 0xff) / PIXEL_FONT_SHADOW_DIVISOR) << 16) +
					((((c->color >> 24) & 0xff) / PIXEL_FONT_SHADOW_DIVISOR) << 24);
			}
		}
	}


}

void pixel_char_print_string(const struct pixel_char* RESTRICT string, int text_size, int line_spacing, int x, int y, int alignment_x, int alignment_y, int max_width, int max_lines, unsigned int* RESTRICT screen, int width, int height, const const void** RESTRICT font_map) {


	if (string->value == '\0') return;

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

			if (string[text_i_line].value != '\x1f' && string[text_i_line].masks & PIXEL_CHAR_BACKGROUND_MASK) pixel_char_background_print(&string[text_i_line], text_size, x_tracer, y_tracer, screen, width, height, font_map);

			x_tracer += (__PIXEL_CHAR_WIDTH(string[text_i_line], font_map) + 1) / 2 * text_size + (__PIXEL_CHAR_WIDTH(string[text_i_line], font_map) > 0 ? text_size : 0);

		}

		x_tracer = x_pos;

		text_i_line = text_i;

		for (; text_i_line <= last_char_line; text_i_line++) {

			if (string[text_i_line].value != '\x1f') pixel_char_print(&string[text_i_line], text_size, x_tracer, y_tracer, screen, width, height, font_map);

			x_tracer += (__PIXEL_CHAR_WIDTH(string[text_i_line], font_map) + 1) / 2 * text_size + (__PIXEL_CHAR_WIDTH(string[text_i_line], font_map) > 0 ? text_size : 0);

		}

		text_i = text_i_line;

		if (string[text_i].value == '\0') break;

		y_tracer += (PIXEL_FONT_RESOULUTION / 2 + line_spacing) * text_size;
		if (string[text_i].value == '\n') text_i++;
	}

#ifdef PIXEL_CHAR_DEBUG

	int i = (alignment_x == ALIGNMENT_LEFT ? x : (alignment_x == ALIGNMENT_RIGHT ? x - max_width : x - max_width / 2)) - 1;

	for (int j = y_pos; j < (PIXEL_FONT_RESOULUTION / 2 + line_spacing) * text_size * lines - line_spacing * text_size + y_pos; j++) {
		if (i >= 0 && i < width && j >= 0 && j < height) screen[i + j * width] = 0xffff7f00;
	}

	i = (alignment_x == ALIGNMENT_LEFT ? x + max_width : (alignment_x == ALIGNMENT_RIGHT ? x : x + max_width / 2));
	for (int j = y_pos; j < (PIXEL_FONT_RESOULUTION / 2 + line_spacing) * text_size * lines - line_spacing * text_size + y_pos; j++) {
		if (i >= 0 && i < width && j >= 0 && j < height) screen[i + j * width] = 0xffff7f00;
	}

	for (int i = x - 2; i <= x + 2; i++) {
		for (int j = y - 2; j <= y + 2; j++) {
			if (i >= 0 && i < width && j >= 0 && j < height) screen[i + j * width] = 0xffff0000;
		}
	}


#endif // _PIXEL_CHAR_DEBUG

}



int pixel_char_get_hover_index(const struct pixel_char* RESTRICT string, int text_size, int line_spacing, int x, int y, int alignment_x, int alignment_y, int max_width, int max_lines, const const void** RESTRICT font_map, int x_hover, int y_hover) {

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



void pixel_char_print_string_gl(const struct pixel_char* RESTRICT string, int text_size, int line_spacing, int x, int y, int alignment_x, int alignment_y, int max_width, int max_lines, unsigned int* RESTRICT screen, int width, int height, const const void** RESTRICT font_map) {


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

	GLCall(glBindBuffer(GL_ARRAY_BUFFER, transform_buffer));
	GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, char_count * 4 * sizeof(int), transforms)); // Update with new vertex data

	GLCall(glBindBuffer(GL_ARRAY_BUFFER, pixel_char_buffer));
	GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, char_count * sizeof(struct pixel_char), pixel_chars)); // Update with new vertex data


	GLCall(glUseProgram(shader_program));

	glUniform1i(width_uniform_location, width);
	glUniform1i(height_uniform_location, height);

	GLCall(glBindVertexArray(VAO));
	GLCall(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, char_count));
	//GLCall(glBindVertexArray(0));


#ifdef PIXEL_CHAR_DEBUG

	int i = (alignment_x == ALIGNMENT_LEFT ? x : (alignment_x == ALIGNMENT_RIGHT ? x - max_width : x - max_width / 2)) - 1;

	for (int j = y_pos; j < (PIXEL_FONT_RESOULUTION / 2 + line_spacing) * text_size * lines - line_spacing * text_size + y_pos; j++) {
		if (i >= 0 && i < width && j >= 0 && j < height) screen[i + j * width] = 0xffff7f00;
	}

	i = (alignment_x == ALIGNMENT_LEFT ? x + max_width : (alignment_x == ALIGNMENT_RIGHT ? x : x + max_width / 2));
	for (int j = y_pos; j < (PIXEL_FONT_RESOULUTION / 2 + line_spacing) * text_size * lines - line_spacing * text_size + y_pos; j++) {
		if (i >= 0 && i < width && j >= 0 && j < height) screen[i + j * width] = 0xffff7f00;
	}

	for (int i = x - 2; i <= x + 2; i++) {
		for (int j = y - 2; j <= y + 2; j++) {
			if (i >= 0 && i < width && j >= 0 && j < height) screen[i + j * width] = 0xffff0000;
		}
	}


#endif // _PIXEL_CHAR_DEBUG

}
