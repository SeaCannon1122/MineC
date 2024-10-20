#include "testing.h"

#include <GL/glew.h>

#include "general/platformlib/opengl_rendering.h"

#include <stdio.h>
#include <stdlib.h>

#include "general/platformlib/platform.h"
#include "pixel_char.h"
#include "key_value.h"
#include "glframebuffer.h"
#include "shader.h"


char* read_text_file(char* filename) {

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


int testing_main() {
	show_console_window();
	printf("Running OpenGL %s\n", glGetString(GL_VERSION));
	GLint maxTextureSize;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	printf("Max texture size: %d\n", maxTextureSize);


	pixel_char_init();

	struct pixel_font* font = load_pixel_font("../../../resources/client/assets/fonts/debug.pixelfont");

	GLuint ssbo;
	GLCall(glGenBuffers(1, &ssbo));
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo));

	
	size_t numHandles = 1024; 
	size_t bufferSize = numHandles * sizeof(GLuint);

	GLCall(glBufferStorage(GL_SHADER_STORAGE_BUFFER, bufferSize, NULL, GL_DYNAMIC_STORAGE_BIT));

	GLuint* initialData = (unsigned int*)calloc(1, bufferSize);

	GLuint font_ssbo;
	GLCall(glGenBuffers(1, &font_ssbo));
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, font_ssbo));
	GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(struct pixel_font), font, GL_STATIC_DRAW));

	GLuint64 font_ssbo_address = glGetBufferDeviceAddress(GL_SHADER_STORAGE_BUFFER, font_ssbo);
	glMakeBufferResidentNV(GL_SHADER_STORAGE_BUFFER, font_ssbo, GL_READ_ONLY);

	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo));
	initialData[0] = font_ssbo_address;
	GLCall(glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, bufferSize, initialData));
	free(initialData);

	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
	GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo));


	pixel_char_convert_string(text_string, "Hello World!\nThis is so cool", 0xff00ffff, 0xffffffff, 0)

	struct frame_buffer fb;

	int width = 700;
	int height = 500;

	frame_buffer_new(&fb, width, height);

	int window = window_create(100, 100, width, height, "NAME");

	width = window_get_width(window);
	height = window_get_width(window);

	unsigned int* pixels = malloc(sizeof(unsigned int) * height * width);

	frame_buffer_bind(&fb);

	GLCall(glClearColor(0.92f, 0.81f, 0.53f, 1.0f));

	GLCall(glViewport(0, 0, width, height));

	while (window_is_active(window) && !get_key_state(KEY_ESCAPE)) {

		int new_width = window_get_width(window);
		int new_height = window_get_height(window);

		if (width != new_width || height != new_height) {
			height = new_height;
			width = new_width;
			free(pixels);
			pixels = malloc(sizeof(unsigned int) * height * width);

			frame_buffer_set_size(&fb, width, height);
			GLCall(glViewport(0, 0, width, height));
		}


		glClear(GL_COLOR_BUFFER_BIT);

		pixel_char_print_string_gl(text_string, 10, 1, width / 2, height / 2, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, 1000000, 0, pixels, width, height, &font);

		GLCall(glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels));

		//pixel_char_print_string(text_string, 5, 1, 300, 300, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, 1000000, 0, pixels, width, height, &font);


		window_draw(window, pixels, width, height, 1);

		sleep_for_ms(5);
	}

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	

	frame_buffer_destroy(&fb);

	window_destroy(window);

}