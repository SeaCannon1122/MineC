#include <stdio.h>
#include <string.h>
#include "utils.h"

#include <window/window.h>
#include <GL/glcorearb.h>
#include <pixelchar/pixelchar.h>
#include <pixelchar/backend/backend_opengl.h>

void DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
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
	//printf("%s %s\n", (type == PIXELCHAR_DEBUG_MESSAGE_TYPE_WARNING ? "[WARNING]" : (type == PIXELCHAR_DEBUG_MESSAGE_TYPE_ERROR ? "[ERROR]" : "[CRITICAL ERROR]")), msg);
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

	window_init_context(NULL);
	window_opengl_load();

	bool a;
	void* window = window_create(100, 100, 200, 200, "window for test", true);
	if (window_glCreateContext(window, 4, 6, NULL, &a) == false) printf("failed to create opengl context\n");
	window_glMakeCurrent(window);

	//window_glSwapIntervalEXT(1);

	PFNGLENABLEPROC glEnable = window_glGetProcAddress("glEnable");
	PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback = window_glGetProcAddress("glDebugMessageCallback");
	PFNGLDEBUGMESSAGECONTROLPROC glDebugMessageControl = window_glGetProcAddress("glDebugMessageControl");
	PFNGLGETSTRINGPROC glGetString = window_glGetProcAddress("glGetString");
	PFNGLBLENDFUNCPROC glBlendFunc = window_glGetProcAddress("glBlendFunc");
	PFNGLCLEARCOLORPROC glClearColor = window_glGetProcAddress("glClearColor");
	PFNGLVIEWPORTPROC glViewport = window_glGetProcAddress("glViewport");
	PFNGLCLEARPROC glClear = window_glGetProcAddress("glClear");

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

	glDebugMessageCallback(DebugCallback, NULL);

	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, NULL, GL_TRUE);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, NULL, GL_TRUE);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, NULL, GL_FALSE);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);

	printf("%s\n", glGetString(GL_RENDERER));
	printf("%s\n", glGetString(GL_VENDOR));
	printf("%s\n", glGetString(GL_VERSION));

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	size_t default_font_data_size;
	void* default_font_data = loadFile("../../../../client/resources/resourcepacks/minec_default/assets/fonts/default.pixelfont", &default_font_data_size);
	if (default_font_data == NULL) printf("failed to load pixelfont\n");

	size_t smooth_font_data_size;
	void* smooth_font_data = loadFile("../../../../client/resources/resourcepacks/minec_default/assets/fonts/smooth.pixelfont", &smooth_font_data_size);
	if (smooth_font_data == NULL) printf("failed to load pixelfont\n");

	PixelcharFont default_font;
	PixelcharResult res = pixelcharFontCreate(default_font_data, default_font_data_size, &default_font);
	PixelcharFont smooth_font;
	res = pixelcharFontCreate(smooth_font_data, smooth_font_data_size, &smooth_font);

	free(default_font_data);
	free(smooth_font_data);

	PixelcharRenderer pcr;
	res = pixelcharRendererCreate(100, &pcr);
	res = pixelcharRendererBackendOpenGLInitialize(pcr, 0, 3, window_glGetProcAddress, 0, 0, 0, 0);
	res = pixelcharRendererBindFont(pcr, default_font, 0);
	res = pixelcharRendererBindFont(pcr, smooth_font, 1);

	pixelcharFontDestroy(default_font);
	pixelcharFontDestroy(smooth_font);

	Pixelchar c[100];

	uint32_t scale = 2;

	

	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;

	window_get_dimensions(window, &width, &height, &x, &y);

	glViewport(0, 0, width, height);
	
	float time = time_get();

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

		float time_now = time_get();

		uint8_t buffer[11];
		snprintf(buffer, sizeof(buffer), "%.10f", time_now - time);

		time = time_now;

		for (uint32_t i = 0; i < 10; i++)
		{
			c[i].character = buffer[i];
			c[i].flags = PIXELCHAR_BACKGROUND_BIT | PIXELCHAR_UNDERLINE_BIT | PIXELCHAR_SHADOW_BIT;
			c[i].fontIndex = i % 2;
			c[i].scale = scale;

			c[i].position[1] = 100;

			if (i == 0) c[i].position[0] = 100;
			else c[i].position[0] = c[i - 1].position[0] + pixelcharGetCharacterRenderingWidth(pcr, &c[i - 1]) + pixelcharGetCharacterRenderingSpacing(pcr, &c[i - 1], &c[i]);

			c[i].color[0] = 0xdc;
			c[i].color[1] = 0xdc;
			c[i].color[2] = 0xdc;
			c[i].color[3] = 255;
			c[i].backgroundColor[0] = 255;
			c[i].backgroundColor[1] = 0;
			c[i].backgroundColor[2] = 0;
			c[i].backgroundColor[3] = 255;

		}


		pixelcharRendererResetQueue(pcr);
		pixelcharRendererEnqueCharacters(pcr, c, 10);

		pixelcharRendererBackendOpenGLRender(pcr, 0, 0, width, height, 4.f, 4.f, 4.f, 1.4f);

		window_glSwapBuffers(window);
	}

	pixelcharRendererBackendOpenGLDeinitialize(pcr, 0);
	pixelcharRendererDestroy(pcr);

	window_glMakeCurrent(NULL);

	window_glDestroyContext(window);
	window_destroy(window);

	window_opengl_unload();
	window_deinit_context();

	return 0;
}