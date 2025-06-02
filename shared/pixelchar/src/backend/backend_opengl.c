#include <pixelchar_internal.h>
#include <pixelchar/backend/backend_opengl.h>

#include <GL/glcorearb.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "opengl_vertex_shader.h"
#include "opengl_fragment_shader.h"

typedef struct _font_backend_opengl
{
	GLuint buffer;
} _font_backend_opengl;

typedef struct _renderer_backend_opengl
{
	struct
	{
		PFNGLGENBUFFERSPROC glGenBuffers;
		PFNGLBINDBUFFERPROC glBindBuffer;
		PFNGLBUFFERDATAPROC glBufferData;
		PFNGLDELETEBUFFERSPROC glDeleteBuffers;
		PFNGLGETERRORPROC glGetError;
		PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
		PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
		PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
		PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer;
		PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
		PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;
		PFNGLCREATESHADERPROC glCreateShader;
		PFNGLSHADERSOURCEPROC glShaderSource;
		PFNGLCOMPILESHADERPROC glCompileShader;
		PFNGLCREATEPROGRAMPROC glCreateProgram;
		PFNGLATTACHSHADERPROC glAttachShader;
		PFNGLLINKPROGRAMPROC glLinkProgram;
		PFNGLGETPROGRAMIVPROC glGetProgramiv;
		PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
		PFNGLDELETESHADERPROC glDeleteShader;
		PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
		PFNGLDELETEPROGRAMPROC glDeleteProgram;
		PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
		PFNGLUSEPROGRAMPROC glUseProgram;
		PFNGLBINDBUFFERBASEPROC glBindBufferBase;
		PFNGLBUFFERSUBDATAPROC glBufferSubData;
		PFNGLUNIFORM2IPROC glUniform2i;
		PFNGLUNIFORM4FPROC glUniform4f;
		PFNGLUNIFORM1UIPROC glUniform1ui;
		PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced;
	} func;

	GLuint vbo;
	GLuint ebo;
	GLuint vao;

	GLuint shader_program;

	GLint uniform_location_screen_size;
	GLint uniform_location_shadow_color_devisor;
	GLint uniform_location_draw_mode;
} _renderer_backend_opengl;

PixelcharResult _font_backend_opengl_add_reference(PixelcharRenderer renderer, uint32_t font_index)
{
	_renderer_backend_opengl* renderer_backend = renderer->backends[PIXELCHAR_BACKEND_OPENGL];

	if (renderer->fonts[font_index]->backends_reference_count[PIXELCHAR_BACKEND_OPENGL] == 0)
	{
		_font_backend_opengl* font_backend = malloc(sizeof(_font_backend_opengl));
		if (font_backend == NULL) return PIXELCHAR_ERROR_OUT_OF_MEMORY;

		memset(font_backend, 0, sizeof(_font_backend_opengl));

		renderer_backend->func.glGenBuffers(1, &font_backend->buffer);
		renderer_backend->func.glBindBuffer(GL_SHADER_STORAGE_BUFFER, font_backend->buffer);

		renderer_backend->func.glBufferData(GL_SHADER_STORAGE_BUFFER, renderer->fonts[font_index]->bitmaps_count * renderer->fonts[font_index]->resolution * renderer->fonts[font_index]->resolution / 8, renderer->fonts[font_index]->bitmaps, GL_STATIC_DRAW);
	
		renderer->fonts[font_index]->backends[PIXELCHAR_BACKEND_OPENGL] = font_backend;
	}

	renderer->fonts[font_index]->backends_reference_count[PIXELCHAR_BACKEND_OPENGL]++;

	return PIXELCHAR_SUCCESS;
}

void _font_backend_opengl_sub_reference(PixelcharRenderer renderer, uint32_t font_index)
{
	_renderer_backend_opengl* renderer_backend = renderer->backends[PIXELCHAR_BACKEND_OPENGL]; 
	_font_backend_opengl* font_backend = renderer->fonts[font_index]->backends[PIXELCHAR_BACKEND_OPENGL];

	if (renderer->fonts[font_index]->backends_reference_count[PIXELCHAR_BACKEND_OPENGL] == 1)
	{
		renderer_backend->func.glDeleteBuffers(1, &font_backend->buffer);

		free(font_backend);
	}

	renderer->fonts[font_index]->backends_reference_count[PIXELCHAR_BACKEND_OPENGL]--;
}

PixelcharResult pixelcharRendererBackendOpenGLInitialize(
	PixelcharRenderer renderer,
	void* (*pfnglGetProcAddress)(uint8_t*),
	uint8_t* vertex_shader_custom,
	uint32_t vertex_shader_custom_length,
	uint8_t* fragment_shader_custom,
	uint32_t fragment_shader_custom_length
)
{
	if (renderer == NULL) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;
	if (renderer->backends[PIXELCHAR_BACKEND_OPENGL] != NULL) return PIXELCHAR_ERROR_BACKEND_ALREADY_INITIALIZED;

	_renderer_backend_opengl* backend = malloc(sizeof(_renderer_backend_opengl));
	if (backend == NULL) return PIXELCHAR_ERROR_OUT_OF_MEMORY;

	memset(backend, 0, sizeof(_renderer_backend_opengl));

	void** functions[] =
	{
		(void**)&backend->func.glGenBuffers,
		(void**)&backend->func.glBindBuffer,
		(void**)&backend->func.glBufferData,
		(void**)&backend->func.glDeleteBuffers,
		(void**)&backend->func.glGetError,
		(void**)&backend->func.glGenVertexArrays,
		(void**)&backend->func.glBindVertexArray,
		(void**)&backend->func.glEnableVertexAttribArray,
		(void**)&backend->func.glVertexAttribIPointer,
		(void**)&backend->func.glVertexAttribPointer,
		(void**)&backend->func.glVertexAttribDivisor,
		(void**)&backend->func.glCreateShader,
		(void**)&backend->func.glShaderSource,
		(void**)&backend->func.glCompileShader,
		(void**)&backend->func.glCreateProgram,
		(void**)&backend->func.glAttachShader,
		(void**)&backend->func.glLinkProgram,
		(void**)&backend->func.glGetProgramiv,
		(void**)&backend->func.glGetProgramInfoLog,
		(void**)&backend->func.glDeleteShader,
		(void**)&backend->func.glGetUniformLocation,
		(void**)&backend->func.glDeleteProgram,
		(void**)&backend->func.glDeleteVertexArrays,
		(void**)&backend->func.glUseProgram,
		(void**)&backend->func.glBindBufferBase,
		(void**)&backend->func.glBufferSubData,
		(void**)&backend->func.glUniform2i,
		(void**)&backend->func.glUniform4f,
		(void**)&backend->func.glUniform1ui,
		(void**)&backend->func.glDrawElementsInstanced
	};

	uint8_t* function_names[] =
	{
		"glGenBuffers",
		"glBindBuffer",
		"glBufferData",
		"glDeleteBuffers",
		"glGetError",
		"glGenVertexArrays",
		"glBindVertexArray",
		"glEnableVertexAttribArray",
		"glVertexAttribIPointer",
		"glVertexAttribPointer",
		"glVertexAttribDivisor",
		"glCreateShader",
		"glShaderSource",
		"glCompileShader",
		"glCreateProgram",
		"glAttachShader",
		"glLinkProgram",
		"glGetProgramiv",
		"glGetProgramInfoLog",
		"glDeleteShader",
		"glGetUniformLocation",
		"glDeleteProgram",
		"glDeleteVertexArrays",
		"glUseProgram",
		"glBindBufferBase",
		"glBufferSubData",
		"glUniform2i",
		"glUniform4f",
		"glUniform1ui",
		"glDrawElementsInstanced"
	};

	for (uint32_t i = 0; i < sizeof(function_names) / sizeof(function_names[0]); i++) *(functions[i]) = (void**)pfnglGetProcAddress(function_names[i]);

	while (backend->func.glGetError() != GL_NO_ERROR);

	backend->func.glGenVertexArrays(1, &backend->vao);
	backend->func.glBindVertexArray(backend->vao);

	backend->func.glGenBuffers(1, &backend->ebo);
	backend->func.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, backend->ebo);

	GLuint indices[] = { 0, 1, 2, 1, 3, 2 };
	backend->func.glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	backend->func.glGenBuffers(1, &backend->vbo);
	backend->func.glBindBuffer(GL_ARRAY_BUFFER, backend->vbo);
	backend->func.glBufferData(GL_ARRAY_BUFFER, renderer->queue_total_length * sizeof(_pixelchar_renderer_char), NULL, GL_DYNAMIC_DRAW);

	backend->func.glEnableVertexAttribArray(0);
	backend->func.glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(_pixelchar_renderer_char), (void*)offsetof(_pixelchar_renderer_char, bitmapIndex));
	backend->func.glVertexAttribDivisor(0, 1);

	backend->func.glEnableVertexAttribArray(1);
	backend->func.glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(_pixelchar_renderer_char), (void*)offsetof(_pixelchar_renderer_char, flags));
	backend->func.glVertexAttribDivisor(1, 1);

	backend->func.glEnableVertexAttribArray(2);
	backend->func.glVertexAttribIPointer(2, 1, GL_UNSIGNED_SHORT, sizeof(_pixelchar_renderer_char), (void*)offsetof(_pixelchar_renderer_char, fontIndex));
	backend->func.glVertexAttribDivisor(2, 1);

	backend->func.glEnableVertexAttribArray(3);
	backend->func.glVertexAttribIPointer(3, 1, GL_UNSIGNED_SHORT, sizeof(_pixelchar_renderer_char), (void*)offsetof(_pixelchar_renderer_char, fontResolution));
	backend->func.glVertexAttribDivisor(3, 1);

	backend->func.glEnableVertexAttribArray(4);
	backend->func.glVertexAttribIPointer(4, 1, GL_UNSIGNED_SHORT, sizeof(_pixelchar_renderer_char), (void*)offsetof(_pixelchar_renderer_char, scale));
	backend->func.glVertexAttribDivisor(4, 1);

	backend->func.glEnableVertexAttribArray(5);
	backend->func.glVertexAttribIPointer(5, 1, GL_UNSIGNED_BYTE, sizeof(_pixelchar_renderer_char), (void*)offsetof(_pixelchar_renderer_char, bitmapWidth));
	backend->func.glVertexAttribDivisor(5, 1);

	backend->func.glEnableVertexAttribArray(6);
	backend->func.glVertexAttribIPointer(6, 2, GL_INT, sizeof(_pixelchar_renderer_char), (void*)offsetof(_pixelchar_renderer_char, position));
	backend->func.glVertexAttribDivisor(6, 1);

	backend->func.glEnableVertexAttribArray(7);
	backend->func.glVertexAttribPointer(7, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(_pixelchar_renderer_char), (void*)offsetof(_pixelchar_renderer_char, color));
	backend->func.glVertexAttribDivisor(7, 1);

	backend->func.glEnableVertexAttribArray(8);
	backend->func.glVertexAttribPointer(8, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(_pixelchar_renderer_char), (void*)offsetof(_pixelchar_renderer_char, backgroundColor));
	backend->func.glVertexAttribDivisor(8, 1);

	backend->func.glBindVertexArray(0);

	size_t pixelchar_opengl_vertex_shader_code_size = sizeof(pixelchar_opengl_vertex_shader_code) - 1;
	size_t pixelchar_opengl_fragment_shader_code_size = sizeof(pixelchar_opengl_fragment_shader_code) - 1;

	GLuint vertex_shader = backend->func.glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_shader = backend->func.glCreateShader(GL_FRAGMENT_SHADER);

	uint8_t* vertex_src = vertex_shader_custom ? vertex_shader_custom : pixelchar_opengl_vertex_shader_code;
	int32_t vertex_src_length = vertex_shader_custom ? vertex_shader_custom_length : pixelchar_opengl_vertex_shader_code_size;

	uint8_t* fragment_src = fragment_shader_custom ? fragment_shader_custom : pixelchar_opengl_fragment_shader_code;
	int32_t fragment_src_length = fragment_shader_custom ? fragment_shader_custom_length : pixelchar_opengl_fragment_shader_code_size;

	backend->func.glShaderSource(vertex_shader, 1, &vertex_src, &vertex_src_length);
	backend->func.glShaderSource(fragment_shader, 1, &fragment_src, &fragment_src_length);

	backend->func.glCompileShader(vertex_shader);
	backend->func.glCompileShader(fragment_shader);

	backend->shader_program = backend->func.glCreateProgram();
	backend->func.glAttachShader(backend->shader_program, vertex_shader);
	backend->func.glAttachShader(backend->shader_program, fragment_shader);
	backend->func.glLinkProgram(backend->shader_program);

	GLint status;
	backend->func.glGetProgramiv(backend->shader_program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		char log[10000];
		backend->func.glGetProgramInfoLog(backend->shader_program, sizeof(log), 0, log);

		printf("Shader Linker Error: %s\n", log);
	}

	backend->func.glDeleteShader(vertex_shader);
	backend->func.glDeleteShader(fragment_shader);

	backend->uniform_location_screen_size = backend->func.glGetUniformLocation(backend->shader_program, "screen_size");
	backend->uniform_location_shadow_color_devisor = backend->func.glGetUniformLocation(backend->shader_program, "shadow_color_devisor");
	backend->uniform_location_draw_mode = backend->func.glGetUniformLocation(backend->shader_program, "draw_mode");

	renderer->backends[PIXELCHAR_BACKEND_OPENGL] = backend;

	for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONT_COUNT; i++)
	{
		if (renderer->fonts[i] != NULL)
		{
			if (_font_backend_opengl_add_reference(renderer, i) == PIXELCHAR_SUCCESS)
				renderer->font_backends_referenced[i][PIXELCHAR_BACKEND_OPENGL] = true;
		}
	}

	return PIXELCHAR_SUCCESS;
}

void pixelcharRendererBackendOpenGLDeinitialize(PixelcharRenderer renderer)
{
	if (renderer == NULL) return;
	if (renderer->backends[PIXELCHAR_BACKEND_OPENGL] == NULL) return;

	_renderer_backend_opengl* backend = renderer->backends[PIXELCHAR_BACKEND_OPENGL];

	for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONT_COUNT; i++)
	{
		if (renderer->fonts[i] != NULL && renderer->font_backends_referenced[i][PIXELCHAR_BACKEND_OPENGL] == true)
			_font_backend_opengl_sub_reference(renderer, i);

		renderer->font_backends_referenced[i][PIXELCHAR_BACKEND_OPENGL] = false;
	}

	backend->func.glDeleteProgram(backend->shader_program);

	backend->func.glDeleteBuffers(1, &backend->vbo);
	backend->func.glDeleteBuffers(1, &backend->ebo);
	backend->func.glDeleteVertexArrays(1, &backend->vao);

	free(backend);
	renderer->backends[PIXELCHAR_BACKEND_OPENGL] = NULL;
}

PixelcharResult pixelcharRendererBackendOpenGLRender(
	PixelcharRenderer renderer,
	uint32_t width,
	uint32_t height,
	float shadowDevisorR,
	float shadowDevisorG,
	float shadowDevisorB,
	float shadowDevisorA
)
{
	if (renderer == NULL) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;
	if (width == 0) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;
	if (height == 0) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;

	if (renderer->queue_filled_length == 0) return PIXELCHAR_SUCCESS;

	if (renderer->backends[PIXELCHAR_BACKEND_OPENGL] == NULL) return PIXELCHAR_ERROR_BACKEND_NOT_INITIALIZED;

	_renderer_backend_opengl* backend = renderer->backends[PIXELCHAR_BACKEND_OPENGL];

	_pixelchar_renderer_convert_queue(renderer, PIXELCHAR_BACKEND_OPENGL);

	backend->func.glUseProgram(backend->shader_program);

	for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONT_COUNT; i++)
	{
		if (renderer->fonts[i]) if (renderer->font_backends_referenced[i][PIXELCHAR_BACKEND_OPENGL] == true) backend->func.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i, ((_font_backend_opengl*)renderer->fonts[i]->backends[PIXELCHAR_BACKEND_OPENGL])->buffer);
	}

	backend->func.glBindBuffer(GL_ARRAY_BUFFER, backend->vbo);
	backend->func.glBufferSubData(GL_ARRAY_BUFFER, 0, renderer->queue_filled_length * sizeof(_pixelchar_renderer_char), renderer->queue);
	backend->func.glBindBuffer(GL_ARRAY_BUFFER, 0);

	backend->func.glBindVertexArray(backend->vao);

	backend->func.glUniform2i(backend->uniform_location_screen_size, width, height);
	backend->func.glUniform4f(backend->uniform_location_shadow_color_devisor, shadowDevisorR, shadowDevisorG, shadowDevisorB, shadowDevisorA);

	for (uint32_t i = 0; i < 3; i++)
	{
		backend->func.glUniform1ui(backend->uniform_location_draw_mode, i);

		backend->func.glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, renderer->queue_filled_length);
	}

	return PIXELCHAR_SUCCESS;
}