#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <window/window.h>
#include <glad/glad.h>
#include <pixelchar/pixelchar.h>

#include "Windows.h"

void GLAPIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar* message, const void* userParam) {
	// Output all debug messages (info, warnings, errors)
	printf("\n\n----OpenGl-Debug-Message-----------------\n");
	printf("  Source: %u\n", source);
	printf("  Type: %u\n", type);
	printf("  ID: %u\n", id);
	printf("  Severity: %u\n", severity);
	printf("  Message: %s\n", message);
	printf("-----------------------------------------\n\n");
}


static void callback(uint32_t type, uint8_t* msg)
{
	printf("%s %s\n", (type == PIXELCHAR_DEBUG_MESSAGE_TYPE_WARNING ? "[WARNING]" : (type == PIXELCHAR_DEBUG_MESSAGE_TYPE_ERROR ? "[ERROR]" : "[CRITICAL ERROR]")), msg);
}

double get_time()
{
	LARGE_INTEGER frequency, start;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&start);

	return (double)1000 * ((double)start.QuadPart / (double)frequency.QuadPart);
}

void* loadFile(uint8_t* src, size_t* size) {

	FILE* file = fopen(src, "rb");
	if (file == NULL) return NULL;

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	rewind(file);

	void* buffer = calloc(1, fileSize);
	if (buffer == NULL) {
		fclose(file);
		return NULL;
	}

	*size = fread(buffer, 1, fileSize, file);

	fclose(file);

	return buffer;
}

int main(int argc, char* argv[]) {

	window_init_system();

	void* window = window_create(100, 100, 200, 200, "window for test", true);
	window_opengl_context_create(window, 4, 6, NULL);
	window_opengl_context_make_current(window);

	window_opengl_set_vsync(true);

	gladLoadGL();

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // Optional: ensures messages are delivered synchronously

	// Register the debug callback function
	glDebugMessageCallback(DebugCallback, NULL);

	// Set the debug message control to allow all messages
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, NULL, GL_TRUE);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, NULL, GL_TRUE);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, NULL, GL_TRUE);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_TRUE);

	printf("%s\n",glGetString(GL_RENDERER));
	printf("%s\n", glGetString(GL_VENDOR));
	printf("%s\n", glGetString(GL_VERSION));

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	pixelchar_set_debug_callback(callback);

	size_t pixelfont_size;
	void* pixelfont = loadFile("runtime_files/assets/MineCdefault/fonts/default.pixelfont", &pixelfont_size);
	if (pixelfont == NULL) printf("failed to load pixelfont\n");

	struct pixelchar_font font;
	pixelchar_font_create(&font, pixelfont, pixelfont_size);

	struct pixelchar_renderer pcr;
	pixelchar_renderer_create(&pcr, 1000);

	size_t vert_length;
	void* vert_src = loadFile("runtime_files/assets/MineCdefault/shaders/pixelchar_gl.vert", &vert_length);
	if (vert_src == NULL) printf("failed to load vert_src\n");
	size_t frag_length;
	void* frag_src = loadFile("runtime_files/assets/MineCdefault/shaders/pixelchar_gl.frag", &frag_length);
	if (frag_src == NULL) printf("failed to load frag_src\n");

	pixelchar_renderer_backend_opengl_init(&pcr, vert_src, vert_length, frag_src, frag_length);

	free(pixelfont);
	free(vert_src);
	free(frag_src);

	pixelchar_renderer_set_font(&pcr, &font, 0);

	uint8_t str[] = { 'H', 'e', 'l', 'l', 'o', ' ', 'W', 'O', 'R', 'L', 'D', '!', 128, 129, 0 };
	uint32_t str_len = sizeof(str) - 1;

	struct pixelchar c[100];

	uint32_t scale = 3;

	for (uint32_t i = 0; i < str_len; i++)
	{
		c[i].value = str[i];
		c[i].masks = PIXELCHAR_MASK_BACKGROUND | PIXELCHAR_MASK_UNDERLINE | PIXELCHAR_MASK_SHADOW;
		c[i].font = 0;
		c[i].scale = scale;

		c[i].position[1] = 100;

		if (i == 0) c[i].position[0] = 100;
		else c[i].position[0] = c[i - 1].position[0] + pixelchar_renderer_get_pixelchar_width(&pcr, &c[i - 1]) + pixelchar_renderer_get_pixelchar_spacing(&pcr, &c[i - 1], &c[i]);

		c[i].color[0] = 0xdc;
		c[i].color[1] = 0xdc;
		c[i].color[2] = 0xdc;
		c[i].color[3] = 255;
		c[i].background_color[0] = 255;
		c[i].background_color[1] = 0;
		c[i].background_color[2] = 0;
		c[i].background_color[3] = 255;

	}

	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;

	window_get_dimensions(window, &width, &height, &x, &y);

	glViewport(0, 0, width, height);

	bool leave = false;
	while (leave == false)
	{
		struct window_event* event;
		while (event = window_next_event(window))
		{
			switch (event->type)
			{

			case WINDOW_EVENT_MOVE_SIZE: {
				printf(
					"New window dimensions:\n  width: %d\n  height: %d\n  position x: %d\n  position y: %d\n\n", 
					event->info.move_size.width, 
					event->info.move_size.height, 
					event->info.move_size.position_x, 
					event->info.move_size.position_y
				);

				width = event->info.move_size.width;
				height = event->info.move_size.height;

				glViewport(0, 0, width, height);

			} break;

			case WINDOW_EVENT_DESTROY: {
				leave = true;
			} break;

			}
		}

		glClear(GL_COLOR_BUFFER_BIT);

		pixelchar_renderer_queue_pixelchars(&pcr, c, str_len);

		pixelchar_renderer_backend_opengl_render(&pcr, width, height, 4.f, 4.f, 4.f, 1.4f);

		window_opengl_swap_buffers(window);
	}

	pixelchar_renderer_backend_opengl_deinit(&pcr);
	pixelchar_renderer_destroy(&pcr);

	pixelchar_font_destroy(&font);

	window_opengl_context_make_current(NULL);

	window_opengl_context_destroy(window);
	window_destroy(window);

	window_deinit_system();

	return 0;
}