#include <stdio.h>
#include <string.h>
#include "utils.h"

#include <cwindow/cwindow.h>
#include <pixelchar/impl/impl_opengl.h>

#define FRAME_COUNT 3
#define VERTEX_BUFFER_LENGTH 128

struct app
{
	cwindow_context* window_context;
	cwindow* window;
	uint32_t width;
	uint32_t height;

	bool leave;

	PFNGLENABLEPROC glEnable;
	PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;
	PFNGLDEBUGMESSAGECONTROLPROC glDebugMessageControl;
	PFNGLGETSTRINGPROC glGetString;
	PFNGLBLENDFUNCPROC glBlendFunc;
	PFNGLCLEARCOLORPROC glClearColor;
	PFNGLVIEWPORTPROC glViewport;
	PFNGLCLEARPROC glClear;
	PFNGLGENBUFFERSPROC glGenBuffers;
	PFNGLDELETEBUFFERSPROC glDeleteBuffers;
	PFNGLBINDBUFFERPROC glBindBuffer;
	PFNGLBUFFERDATAPROC glBufferData;
	PFNGLBUFFERSUBDATAPROC glBufferSubData;

	GLuint vertex_buffer;

	PixelcharImplOpenGLFactory factory;
	PixelcharImplOpenGLRenderer renderer;
	PixelcharImplOpenGLFont font;

};

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

void event_callback(cwindow* window, const cwindow_event* event, struct app* app)
{
	switch (event->type)
	{

	case CWINDOW_EVENT_SIZE: {
		printf(
			"New window dimensions:\n  width: %d\n  height: %d\n  position x: %d\n  position y: %d\n\n",
			event->info.size.width,
			event->info.size.height
		);

		app->width = event->info.size.width;
		app->height = event->info.size.height;

		app->glViewport(0, 0, app->width, app->height);

	} break;

	case CWINDOW_EVENT_DESTROY: {
		app->leave = true;
	} break;
	
	}
}

void* loadFile(uint8_t* src, size_t* size)
{

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

void pixelchar_callback(uint8_t* message, void* userparam)
{
	printf("%s\n", message);
}

int main(int argc, char* argv[])
{
	struct app app;
	app.width = 200;
	app.height = 200;

	app.window_context = cwindow_context_create("context");
	cwindow_context_graphics_opengl_load(app.window_context);

	bool a;
	app.window = cwindow_create(app.window_context, 100, 100, app.width, app.height, "window for test", true, event_callback);
	cwindow_set_event_callback_user_parameter(app.window, &app);

	if (cwindow_glCreateContext(app.window, 4, 6, NULL, &a) == false) printf("failed to create opengl context\n");
	cwindow_glMakeCurrent(app.window_context, app.window);

	cwindow_glSwapIntervalEXT(app.window, 0);

	app.glEnable = cwindow_glGetProcAddress(app.window, "glEnable");
	app.glDebugMessageCallback = cwindow_glGetProcAddress(app.window, "glDebugMessageCallback");
	app.glDebugMessageControl = cwindow_glGetProcAddress(app.window, "glDebugMessageControl");
	app.glGetString = cwindow_glGetProcAddress(app.window, "glGetString");
	app.glBlendFunc = cwindow_glGetProcAddress(app.window, "glBlendFunc");
	app.glClearColor = cwindow_glGetProcAddress(app.window, "glClearColor");
	app.glViewport = cwindow_glGetProcAddress(app.window, "glViewport");
	app.glClear = cwindow_glGetProcAddress(app.window, "glClear");
	app.glGenBuffers = cwindow_glGetProcAddress(app.window, "glGenBuffers");
	app.glDeleteBuffers = cwindow_glGetProcAddress(app.window, "glDeleteBuffers");
	app.glBindBuffer = cwindow_glGetProcAddress(app.window, "glBindBuffer");
	app.glBufferSubData = cwindow_glGetProcAddress(app.window, "glBufferSubData");
	app.glBufferData = cwindow_glGetProcAddress(app.window, "glBufferData");

	app.glEnable(GL_DEBUG_OUTPUT);
	app.glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

	app.glDebugMessageCallback(DebugCallback, NULL);

	app.glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, NULL, GL_TRUE);
	app.glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, NULL, GL_TRUE);
	app.glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, NULL, GL_FALSE);
	app.glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);

	printf("%s\n", app.glGetString(GL_RENDERER));
	printf("%s\n", app.glGetString(GL_VENDOR));
	printf("%s\n", app.glGetString(GL_VERSION));

	app.glEnable(GL_BLEND);
	app.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	app.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	size_t default_font_data_size;
	void* default_font_data = loadFile("../../../../shared/pixelchar/pixelfonts/debug_font/debug.pixelfont", &default_font_data_size);
	if (default_font_data == NULL) printf("failed to load pixelfont\n");

	size_t vertex_shader_size, fragment_shader_size;
	void* vertex_shader = loadFile("../../../../client/resources/shaders/gui_text/gui_text_gl.vert", &vertex_shader_size);
	void* fragment_shader = loadFile("../../../../client/resources/shaders/gui_text/gui_text_gl.frag", &fragment_shader_size);

	PixelcharFont* default_font;
	PixelcharResult res = pixelcharFontLoadFromFileData(default_font_data, default_font_data_size, &default_font, pixelchar_callback, NULL);

	app.glGenBuffers(1, &app.vertex_buffer);
	app.glBindBuffer(GL_ARRAY_BUFFER, app.vertex_buffer);
	app.glBufferData(GL_ARRAY_BUFFER, sizeof(Pixelchar) * VERTEX_BUFFER_LENGTH * FRAME_COUNT, NULL, GL_DYNAMIC_DRAW);

	PixelcharImplOpenGLFactoryCreate(cwindow_glGetProcAddress, app.window, &app.factory, pixelchar_callback, NULL);
	PixelcharImplOpenGLRendererCreate(&app.factory, vertex_shader, vertex_shader_size, fragment_shader, fragment_shader_size, &app.renderer, pixelchar_callback, NULL);
	PixelcharImplOpenGLFontCreate(&app.factory, default_font, &app.font, pixelchar_callback, NULL);

	free(default_font_data);
	free(vertex_shader);
	free(fragment_shader);

	Pixelchar c[VERTEX_BUFFER_LENGTH];

	uint32_t scale = 2;	
	
	double time = time_get();

	uint32_t frame_index = 0;

	app.leave = false;
	while (app.leave == false)
	{
		cwindow_handle_events(app.window);
		app.glViewport(0, 0, app.width, app.height);
		app.glClear(GL_COLOR_BUFFER_BIT);

		double time_now = time_get();

		uint8_t buffer[VERTEX_BUFFER_LENGTH];
		//snprintf(buffer, sizeof(buffer), "  %.10f", time_now - time);


		time = time_now;

		for (uint32_t i = 0; i < VERTEX_BUFFER_LENGTH; i++)
		{
			pixelcharFill(
				(i % 32 == 0 ? 100 : c[i - 1].position[0] + c[i - 1].width),
				(i == 0 ? 100 : (i % 32 == 0 ? c[i - 1].position[1] + 100 : c[i - 1].position[1])),
				i,
				3,
				PIXELCHAR_MODIFIER_BACKGROUND_BIT | PIXELCHAR_MODIFIER_UNDERLINE_BIT | PIXELCHAR_MODIFIER_SHADOW_BIT,
				0xffdcdcdc,
				i%2 ? 0xff00ffff : 0xff0000ff,
				default_font,
				&c[i]
			);
		}


		app.glBindBuffer(GL_ARRAY_BUFFER, app.vertex_buffer);
		app.glBufferSubData(GL_ARRAY_BUFFER, sizeof(Pixelchar) * VERTEX_BUFFER_LENGTH * frame_index, sizeof(Pixelchar) * VERTEX_BUFFER_LENGTH, c);

		PixelcharImplOpenGLRender(&app.renderer, VERTEX_BUFFER_LENGTH, app.vertex_buffer, sizeof(Pixelchar) * VERTEX_BUFFER_LENGTH * frame_index, &app.font, app.width, app.height, 4.f, 4.f, 4.f, 1.4f);

		cwindow_glSwapBuffers(app.window);

		frame_index = (frame_index + 1 ) % 2;
	}

	PixelcharImplOpenGLFontDestroy(&app.font);
	PixelcharImplOpenGLRendererDestroy(&app.renderer);
	PixelcharImplOpenGLFactoryDestroy(&app.factory);

	app.glDeleteBuffers(1, &app.vertex_buffer);

	free(default_font);

	cwindow_glMakeCurrent(app.window_context, app.window);

	cwindow_glDestroyContext(app.window);
	cwindow_destroy(app.window);

	cwindow_context_graphics_opengl_unload(app.window_context);
	cwindow_context_destroy(app.window_context);

	return 0;
}