#include <stdio.h>
#include <string.h>
#include "utils.h"

#include <cwindow/cwindow.h>
#include <pixelchar/impl/impl_opengl.h>

#define FRAME_COUNT 3

extern cwindow_context* window_context;
extern cwindow* window;
extern uint32_t width;
extern uint32_t height;

extern PixelcharFont* default_font;
extern Pixelchar c[128];

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

uint32_t frame_index = 0;

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


void pixelchar_callback(uint8_t* message, void* userparam);

void pixelchar_impl_init()
{
	cwindow_context_graphics_opengl_load(window_context);
	bool a;
	if (cwindow_glCreateContext(window, 4, 6, NULL, &a) == false) printf("failed to create opengl context\n");
	cwindow_glMakeCurrent(window_context, window);

	cwindow_glSwapIntervalEXT(window, 0);

	glEnable = cwindow_glGetProcAddress(window, "glEnable");
	glDebugMessageCallback = cwindow_glGetProcAddress(window, "glDebugMessageCallback");
	glDebugMessageControl = cwindow_glGetProcAddress(window, "glDebugMessageControl");
	glGetString = cwindow_glGetProcAddress(window, "glGetString");
	glBlendFunc = cwindow_glGetProcAddress(window, "glBlendFunc");
	glClearColor = cwindow_glGetProcAddress(window, "glClearColor");
	glViewport = cwindow_glGetProcAddress(window, "glViewport");
	glClear = cwindow_glGetProcAddress(window, "glClear");
	glGenBuffers = cwindow_glGetProcAddress(window, "glGenBuffers");
	glDeleteBuffers = cwindow_glGetProcAddress(window, "glDeleteBuffers");
	glBindBuffer = cwindow_glGetProcAddress(window, "glBindBuffer");
	glBufferSubData = cwindow_glGetProcAddress(window, "glBufferSubData");
	glBufferData = cwindow_glGetProcAddress(window, "glBufferData");

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

	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Pixelchar) * 128 * FRAME_COUNT, NULL, GL_DYNAMIC_DRAW);

	PixelcharImplOpenGLFactoryCreate(cwindow_glGetProcAddress, window, &factory, pixelchar_callback, NULL);
	PixelcharImplOpenGLRendererCreate(&factory, NULL, 0, NULL, 0, &renderer, pixelchar_callback, NULL);
	PixelcharImplOpenGLFontCreate(&factory, default_font, &font, pixelchar_callback, NULL);
}

void pixelchar_impl_frame()
{
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(Pixelchar) * 128 * frame_index, sizeof(Pixelchar) * 128, c);

	PixelcharImplOpenGLRender(&renderer, 128, vertex_buffer, sizeof(Pixelchar) * 128 * frame_index, &font, width, height, 4.f, 4.f, 4.f, 1.4f);

	cwindow_glSwapBuffers(window);

	frame_index = (frame_index + 1) % 2;
}

void pixelchar_impl_deinit()
{
	PixelcharImplOpenGLFontDestroy(&font);
	PixelcharImplOpenGLRendererDestroy(&renderer);
	PixelcharImplOpenGLFactoryDestroy(&factory);

	glDeleteBuffers(1, &vertex_buffer);

	cwindow_glMakeCurrent(window_context, window);
	cwindow_glDestroyContext(window);
	cwindow_context_graphics_opengl_unload(window_context);

	return 0;
}