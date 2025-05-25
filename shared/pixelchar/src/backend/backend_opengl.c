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

		glGenBuffers(1, &font_backend->buffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, font_backend->buffer);

		glBufferData(GL_SHADER_STORAGE_BUFFER, renderer->fonts[font_index]->bitmaps_count * renderer->fonts[font_index]->resolution * renderer->fonts[font_index]->resolution / 8, renderer->fonts[font_index]->bitmaps, GL_STATIC_DRAW);
	
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
		glDeleteBuffers(1, &font_backend->buffer);

		free(font_backend);
	}

	renderer->fonts[font_index]->backends_reference_count[PIXELCHAR_BACKEND_OPENGL]--;
}

PixelcharResult pixelcharRendererBackendOpenGLInitialize(
	PixelcharRenderer renderer,
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

	while (glGetError() != GL_NO_ERROR);

	glGenVertexArrays(1, &backend->vao);
	glBindVertexArray(backend->vao);

	glGenBuffers(1, &backend->ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, backend->ebo);

	GLuint indices[] = { 0, 1, 2, 1, 3, 2 };
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glGenBuffers(1, &backend->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, backend->vbo);
	glBufferData(GL_ARRAY_BUFFER, renderer->queue_total_length * sizeof(_pixelchar_renderer_char), NULL, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(_pixelchar_renderer_char), (void*)offsetof(_pixelchar_renderer_char, bitmapIndex));
	glVertexAttribDivisor(0, 1);

	glEnableVertexAttribArray(1);
	glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(_pixelchar_renderer_char), (void*)offsetof(_pixelchar_renderer_char, flags));
	glVertexAttribDivisor(1, 1);

	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(2, 1, GL_UNSIGNED_SHORT, sizeof(_pixelchar_renderer_char), (void*)offsetof(_pixelchar_renderer_char, fontIndex));
	glVertexAttribDivisor(2, 1);

	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 1, GL_UNSIGNED_SHORT, sizeof(_pixelchar_renderer_char), (void*)offsetof(_pixelchar_renderer_char, fontResolution));
	glVertexAttribDivisor(3, 1);

	glEnableVertexAttribArray(4);
	glVertexAttribIPointer(4, 1, GL_UNSIGNED_SHORT, sizeof(_pixelchar_renderer_char), (void*)offsetof(_pixelchar_renderer_char, scale));
	glVertexAttribDivisor(4, 1);

	glEnableVertexAttribArray(5);
	glVertexAttribIPointer(5, 1, GL_UNSIGNED_BYTE, sizeof(_pixelchar_renderer_char), (void*)offsetof(_pixelchar_renderer_char, bitmapWidth));
	glVertexAttribDivisor(5, 1);

	glEnableVertexAttribArray(6);
	glVertexAttribIPointer(6, 2, GL_INT, sizeof(_pixelchar_renderer_char), (void*)offsetof(_pixelchar_renderer_char, position));
	glVertexAttribDivisor(6, 1);

	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(_pixelchar_renderer_char), (void*)offsetof(_pixelchar_renderer_char, color));
	glVertexAttribDivisor(7, 1);

	glEnableVertexAttribArray(8);
	glVertexAttribPointer(8, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(_pixelchar_renderer_char), (void*)offsetof(_pixelchar_renderer_char, backgroundColor));
	glVertexAttribDivisor(8, 1);

	glBindVertexArray(0);

	size_t pixelchar_opengl_vertex_shader_code_size = sizeof(pixelchar_opengl_vertex_shader_code) - 1;
	size_t pixelchar_opengl_fragment_shader_code_size = sizeof(pixelchar_opengl_fragment_shader_code) - 1;

	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	uint8_t* vertex_src = vertex_shader_custom ? vertex_shader_custom : pixelchar_opengl_vertex_shader_code;
	int32_t vertex_src_length = vertex_shader_custom ? vertex_shader_custom_length : pixelchar_opengl_vertex_shader_code_size;

	uint8_t* fragment_src = fragment_shader_custom ? fragment_shader_custom : pixelchar_opengl_fragment_shader_code;
	int32_t fragment_src_length = fragment_shader_custom ? fragment_shader_custom_length : pixelchar_opengl_fragment_shader_code_size;

	glShaderSource(vertex_shader, 1, &vertex_src, &vertex_src_length);
	glShaderSource(fragment_shader, 1, &fragment_src, &fragment_src_length);

	glCompileShader(vertex_shader);
	glCompileShader(fragment_shader);

	backend->shader_program = glCreateProgram();
	glAttachShader(backend->shader_program, vertex_shader);
	glAttachShader(backend->shader_program, fragment_shader);
	glLinkProgram(backend->shader_program);

	GLint status;
	glGetProgramiv(backend->shader_program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		char log[10000];
		glGetProgramInfoLog(backend->shader_program, sizeof(log), 0, log);

		printf("Shader Linker Error: %s\n", log);
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	backend->uniform_location_screen_size = glGetUniformLocation(backend->shader_program, "screen_size");
	backend->uniform_location_shadow_color_devisor = glGetUniformLocation(backend->shader_program, "shadow_color_devisor");
	backend->uniform_location_draw_mode = glGetUniformLocation(backend->shader_program, "draw_mode");

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

	glDeleteProgram(backend->shader_program);

	glDeleteBuffers(1, &backend->vbo);
	glDeleteBuffers(1, &backend->ebo);
	glDeleteVertexArrays(1, &backend->vao);

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

	if (renderer->queue_filled_length == 0) return;

	if (renderer->backends[PIXELCHAR_BACKEND_OPENGL] == NULL) return PIXELCHAR_ERROR_BACKEND_NOT_INITIALIZED;

	_renderer_backend_opengl* backend = renderer->backends[PIXELCHAR_BACKEND_OPENGL];

	_pixelchar_renderer_convert_queue(renderer, PIXELCHAR_BACKEND_OPENGL);

	glUseProgram(backend->shader_program);

	for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONT_COUNT; i++)
	{
		if (renderer->fonts[i]) if (renderer->font_backends_referenced[i][PIXELCHAR_BACKEND_OPENGL] == true) glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i, ((_font_backend_opengl*)renderer->fonts[i]->backends[PIXELCHAR_BACKEND_OPENGL])->buffer);
	}

	glBindVertexArray(backend->vao);

	glBindBuffer(GL_ARRAY_BUFFER, backend->vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, renderer->queue_filled_length * sizeof(_pixelchar_renderer_char), renderer->queue);

	glUniform2i(backend->uniform_location_screen_size, width, height);
	glUniform4f(backend->uniform_location_shadow_color_devisor, shadowDevisorR, shadowDevisorG, shadowDevisorB, shadowDevisorA);

	for (uint32_t i = 0; i < 3; i++)
	{
		glUniform1ui(backend->uniform_location_draw_mode, i);

		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, renderer->queue_filled_length);
	}

	renderer->queue_filled_length = 0;
}