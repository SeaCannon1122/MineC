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

	void* buffer = malloc(fileSize);
	if (buffer == NULL) {
		fclose(file);
		return NULL;
	}

	size_t bytesRead = fread(buffer, 1, fileSize, file);
	fclose(file);
	*size = fileSize;

	return buffer;
}

struct pixel_font* load_pixel_font(char* src) {

	FILE* file = fopen(src, "rb");
	if (file == NULL) return NULL;

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	rewind(file);

	void* buffer = malloc(sizeof(struct pixel_font));
	if (buffer == NULL) {
		fclose(file);
		return NULL;
	}

	size_t bytesRead = fread(buffer, 1, fileSize, file);

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

	int y_pos = y - (alignment_y == ALIGNMENT_TOP ? 0 : (alignment_y == ALIGNMENT_BOTTOM ? (PIXEL_FONT_RESOLUTION / 2 + line_spacing) * text_size * lines - line_spacing * text_size : ((PIXEL_FONT_RESOLUTION / 2 + line_spacing) * text_size * lines - line_spacing * text_size) / 2));

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

			if (string[text_i_line].value != '\x1f') if (x_hover >= x_tracer - (text_size + 1) / 2 && x_hover < x_tracer + (__PIXEL_CHAR_WIDTH(string[text_i_line], font_map) + 1) / 2 * text_size + text_size / 2 && y_hover >= y_tracer && y_hover < y_tracer + PIXEL_FONT_RESOLUTION * text_size / 2) return text_i_line;

			x_tracer += (__PIXEL_CHAR_WIDTH(string[text_i_line], font_map) + 1) / 2 * text_size + (__PIXEL_CHAR_WIDTH(string[text_i_line], font_map) > 0 ? text_size : 0);

		}

		text_i = text_i_line;

		if (string[text_i].value == '\0') break;

		y_tracer += (PIXEL_FONT_RESOLUTION / 2 + line_spacing) * text_size;
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

	int y_pos = y - (alignment_y == ALIGNMENT_TOP ? 0 : (alignment_y == ALIGNMENT_BOTTOM ? (PIXEL_FONT_RESOLUTION / 2 + line_spacing) * text_size * lines - line_spacing * text_size : ((PIXEL_FONT_RESOLUTION / 2 + line_spacing) * text_size * lines - line_spacing * text_size) / 2));

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

		y_tracer += (PIXEL_FONT_RESOLUTION / 2 + line_spacing) * text_size;
		if (string[text_i].value == '\n') text_i++;
	}

}
