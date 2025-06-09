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
		PFNGLBUFFERSUBDATAPROC glBufferSubData;
		PFNGLDELETEBUFFERSPROC glDeleteBuffers;
		PFNGLGETERRORPROC glGetError;
		PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
		PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
		PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
		PFNGLVERTEXATTRIBIFORMATPROC glVertexAttribIFormat;
		PFNGLVERTEXATTRIBFORMATPROC glVertexAttribFormat;
		PFNGLVERTEXATTRIBBINDINGPROC glVertexAttribBinding;
		PFNGLVERTEXBINDINGDIVISORPROC glVertexBindingDivisor;
		PFNGLCREATESHADERPROC glCreateShader;
		PFNGLSHADERSOURCEPROC glShaderSource;
		PFNGLCOMPILESHADERPROC glCompileShader;
		PFNGLCREATEPROGRAMPROC glCreateProgram;
		PFNGLATTACHSHADERPROC glAttachShader;
		PFNGLLINKPROGRAMPROC glLinkProgram;
		PFNGLGETSHADERIVPROC glGetShaderiv;
		PFNGLGETPROGRAMIVPROC glGetProgramiv;
		PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
		PFNGLDELETESHADERPROC glDeleteShader;
		PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
		PFNGLDELETEPROGRAMPROC glDeleteProgram;
		PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
		PFNGLUSEPROGRAMPROC glUseProgram;
		PFNGLBINDBUFFERBASEPROC glBindBufferBase;
		PFNGLBINDVERTEXBUFFERPROC glBindVertexBuffer;
		PFNGLUNIFORM2IPROC glUniform2i;
		PFNGLUNIFORM4FPROC glUniform4f;
		PFNGLUNIFORM1UIPROC glUniform1ui;
		PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced;
	} func;

	GLuint vbos[PIXELCHAR_BACKEND_OPENGL_MAX_RESOURCE_FRAME_COUNT];
	GLuint ebo;
	GLuint vao;
	uint32_t resource_frame_count;

	GLuint shader_program;

	GLint uniform_location_screen_size;
	GLint uniform_location_shadow_color_devisor;
	GLint uniform_location_draw_mode;
} _renderer_backend_opengl;


PixelcharResult _font_backend_opengl_add_reference(PixelcharRenderer renderer, uint32_t font_index, uint32_t backend_slot_index)
{
	_renderer_backend_opengl* renderer_backend = renderer->backends[backend_slot_index].data;

	if (renderer->fonts[font_index]->backends_reference_count[backend_slot_index] == 0)
	{
		_font_backend_opengl* font_backend = calloc(1, sizeof(_font_backend_opengl));
		if (font_backend == NULL) return PIXELCHAR_ERROR_OUT_OF_MEMORY;

		renderer_backend->func.glGenBuffers(1, &font_backend->buffer);
		renderer_backend->func.glBindBuffer(GL_SHADER_STORAGE_BUFFER, font_backend->buffer);

		renderer_backend->func.glBufferData(GL_SHADER_STORAGE_BUFFER, renderer->fonts[font_index]->bitmaps_count * renderer->fonts[font_index]->resolution * renderer->fonts[font_index]->resolution / 8, renderer->fonts[font_index]->bitmaps, GL_STATIC_DRAW);
	
		renderer->fonts[font_index]->backends[backend_slot_index] = font_backend;
	}

	renderer->fonts[font_index]->backends_reference_count[backend_slot_index]++;

	return PIXELCHAR_SUCCESS;
}

void _font_backend_opengl_sub_reference(PixelcharRenderer renderer, uint32_t font_index, uint32_t backend_slot_index)
{
	_renderer_backend_opengl* renderer_backend = renderer->backends[backend_slot_index].data;
	_font_backend_opengl* font_backend = renderer->fonts[font_index]->backends[backend_slot_index];

	if (renderer->fonts[font_index]->backends_reference_count[backend_slot_index] == 1)
	{
		renderer_backend->func.glDeleteBuffers(1, &font_backend->buffer);

		free(font_backend);
	}

	renderer->fonts[font_index]->backends_reference_count[backend_slot_index]--;
}

PixelcharResult _get_status(_renderer_backend_opengl* backend)
{
	uint32_t result = PIXELCHAR_SUCCESS;
	for (uint32_t i = 0; backend->func.glGetError() != GL_NO_ERROR && i < 32; i++) result = PIXELCHAR_ERROR_BACKEND_API;
	return result;
}

PixelcharResult pixelcharRendererBackendOpenGLInitialize(
	PixelcharRenderer renderer,
	uint32_t backendSlotIndex,
	uint32_t resourceFrameCount,
	void* (*pfnglGetProcAddress)(uint8_t*),
	uint8_t* vertex_shader_custom,
	uint32_t vertex_shader_custom_length,
	uint8_t* fragment_shader_custom,
	uint32_t fragment_shader_custom_length
)
{
	if (renderer == NULL) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;
	if (backendSlotIndex >= PIXELCHAR_RENDERER_MAX_BACKEND_COUNT) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;
	if (resourceFrameCount >= PIXELCHAR_BACKEND_OPENGL_MAX_RESOURCE_FRAME_COUNT) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;
	if (pfnglGetProcAddress == NULL) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;
	if (renderer->backends[backendSlotIndex].data != NULL) return PIXELCHAR_ERROR_BACKEND_SLOT_ALREADY_IN_USED;

	uint32_t result = PIXELCHAR_SUCCESS;

	bool
		backend_memory_allocated = false,
		functions_retrieved = false,
		vao_created = false,
		ebo_created = false,
		vbos_created = false,
		vertex_shader_created = false,
		fragment_shader_created = false,
		program_created = false
	;

	_renderer_backend_opengl* backend;
	if ((backend = calloc(1, sizeof(_renderer_backend_opengl))) == NULL) result = PIXELCHAR_ERROR_OUT_OF_MEMORY;
	else backend_memory_allocated = true;

	if (result == PIXELCHAR_SUCCESS)
	{
		backend->resource_frame_count = resourceFrameCount;

		struct load_entry { void** load_dst; uint8_t* func_name; };
#define LOAD_FUNC_ENTRY(func_name) {(void**)&backend->func.func_name, #func_name}

		struct load_entry load_entries[] =
		{
			LOAD_FUNC_ENTRY(glGenBuffers),
			LOAD_FUNC_ENTRY(glBindBuffer),
			LOAD_FUNC_ENTRY(glBufferData),
			LOAD_FUNC_ENTRY(glBufferSubData),
			LOAD_FUNC_ENTRY(glDeleteBuffers),
			LOAD_FUNC_ENTRY(glGetError),
			LOAD_FUNC_ENTRY(glGenVertexArrays),
			LOAD_FUNC_ENTRY(glBindVertexArray),
			LOAD_FUNC_ENTRY(glEnableVertexAttribArray),
			LOAD_FUNC_ENTRY(glVertexAttribIFormat),
			LOAD_FUNC_ENTRY(glVertexAttribFormat),
			LOAD_FUNC_ENTRY(glVertexAttribBinding),
			LOAD_FUNC_ENTRY(glVertexBindingDivisor),
			LOAD_FUNC_ENTRY(glCreateShader),
			LOAD_FUNC_ENTRY(glShaderSource),
			LOAD_FUNC_ENTRY(glCompileShader),
			LOAD_FUNC_ENTRY(glCreateProgram),
			LOAD_FUNC_ENTRY(glAttachShader),
			LOAD_FUNC_ENTRY(glLinkProgram),
			LOAD_FUNC_ENTRY(glGetShaderiv),
			LOAD_FUNC_ENTRY(glGetProgramiv),
			LOAD_FUNC_ENTRY(glGetProgramInfoLog),
			LOAD_FUNC_ENTRY(glDeleteShader),
			LOAD_FUNC_ENTRY(glGetUniformLocation),
			LOAD_FUNC_ENTRY(glDeleteProgram),
			LOAD_FUNC_ENTRY(glDeleteVertexArrays),
			LOAD_FUNC_ENTRY(glUseProgram),
			LOAD_FUNC_ENTRY(glBindBufferBase),
			LOAD_FUNC_ENTRY(glBindVertexBuffer),
			LOAD_FUNC_ENTRY(glUniform2i),
			LOAD_FUNC_ENTRY(glUniform4f),
			LOAD_FUNC_ENTRY(glUniform1ui),
			LOAD_FUNC_ENTRY(glDrawElementsInstanced)
		};

		for (uint32_t i = 0; i < sizeof(load_entries) / sizeof(load_entries[0]); i++) if ((*load_entries[i].load_dst = (void**)pfnglGetProcAddress(load_entries[i].func_name)) == NULL)
		{
			result = PIXELCHAR_ERROR_BACKEND_API;
			break;
		}
		functions_retrieved = true;
	}

	if (functions_retrieved) _get_status(backend);

	if (result == PIXELCHAR_SUCCESS)
	{
		backend->func.glGenVertexArrays(1, &backend->vao);

		if (backend->vao == 0) result = PIXELCHAR_ERROR_BACKEND_API;
		else vao_created = true;
		_get_status(backend);
	}
	if (result == PIXELCHAR_SUCCESS)
	{
		backend->func.glBindVertexArray(backend->vao);
		result = _get_status(backend);
	}
	if (result == PIXELCHAR_SUCCESS)
	{
		backend->func.glGenBuffers(1, &backend->ebo);

		if (backend->ebo == 0) result = PIXELCHAR_ERROR_BACKEND_API;
		else ebo_created = true;
		_get_status(backend);
	}
	if (result == PIXELCHAR_SUCCESS)
	{
		backend->func.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, backend->ebo);
		result = _get_status(backend);
	}
	if (result == PIXELCHAR_SUCCESS)
	{
		GLuint indices[] = { 0, 1, 2, 1, 3, 2 };
		backend->func.glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		result = _get_status(backend);
	}

	if (result == PIXELCHAR_SUCCESS)
	{
		backend->func.glEnableVertexAttribArray(0);
		backend->func.glVertexAttribIFormat(0, 1, GL_UNSIGNED_INT, 0);
		backend->func.glVertexAttribBinding(0, 0);
		backend->func.glVertexBindingDivisor(0, 1);

		backend->func.glEnableVertexAttribArray(1);
		backend->func.glVertexAttribIFormat(1, 1, GL_UNSIGNED_INT, offsetof(_pixelchar_renderer_char, flags));
		backend->func.glVertexAttribBinding(1, 0);
		backend->func.glVertexBindingDivisor(1, 1);

		backend->func.glEnableVertexAttribArray(2);
		backend->func.glVertexAttribIFormat(2, 1, GL_UNSIGNED_SHORT, offsetof(_pixelchar_renderer_char, fontIndex));
		backend->func.glVertexAttribBinding(2, 0);
		backend->func.glVertexBindingDivisor(2, 1);

		backend->func.glEnableVertexAttribArray(3);
		backend->func.glVertexAttribIFormat(3, 1, GL_UNSIGNED_SHORT, offsetof(_pixelchar_renderer_char, fontResolution));
		backend->func.glVertexAttribBinding(3, 0);
		backend->func.glVertexBindingDivisor(3, 1);

		backend->func.glEnableVertexAttribArray(4);
		backend->func.glVertexAttribIFormat(4, 1, GL_UNSIGNED_SHORT, offsetof(_pixelchar_renderer_char, scale));
		backend->func.glVertexAttribBinding(4, 0);
		backend->func.glVertexBindingDivisor(4, 1);

		backend->func.glEnableVertexAttribArray(5);
		backend->func.glVertexAttribIFormat(5, 1, GL_UNSIGNED_BYTE, offsetof(_pixelchar_renderer_char, bitmapWidth));
		backend->func.glVertexAttribBinding(5, 0);
		backend->func.glVertexBindingDivisor(5, 1);

		backend->func.glEnableVertexAttribArray(6);
		backend->func.glVertexAttribIFormat(6, 2, GL_INT, offsetof(_pixelchar_renderer_char, position));
		backend->func.glVertexAttribBinding(6, 0);
		backend->func.glVertexBindingDivisor(6, 1);

		backend->func.glEnableVertexAttribArray(7);
		backend->func.glVertexAttribFormat(7, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(_pixelchar_renderer_char, color));
		backend->func.glVertexAttribBinding(7, 0);
		backend->func.glVertexBindingDivisor(7, 1);

		backend->func.glEnableVertexAttribArray(8);
		backend->func.glVertexAttribFormat(8, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(_pixelchar_renderer_char, backgroundColor));
		backend->func.glVertexAttribBinding(8, 0);
		backend->func.glVertexBindingDivisor(8, 1);

		result = _get_status(backend);
	}

	if (functions_retrieved) backend->func.glBindVertexArray(0);

	if (result == PIXELCHAR_SUCCESS)
	{
		backend->func.glGenBuffers(backend->resource_frame_count, backend->vbos);

		bool some_buffer_failed = false;
		for (uint32_t i = 0; i < backend->resource_frame_count; i++) if (backend->vbos[i] == 0) some_buffer_failed = true;
		if (some_buffer_failed)
		{
			for (uint32_t i = 0; i < backend->resource_frame_count; i++) if (backend->vbos[i] != 0) backend->func.glDeleteBuffers(1, &backend->vbos[i]);
			_get_status(backend);
		}
		else vbos_created = true;
	}

	for (uint32_t i = 0; i < backend->resource_frame_count && result == PIXELCHAR_SUCCESS; i++)
	{
		backend->func.glBindBuffer(GL_ARRAY_BUFFER, backend->vbos[i]);
		if ((result = _get_status(backend)) == PIXELCHAR_SUCCESS)
		{
			backend->func.glBufferData(GL_ARRAY_BUFFER, renderer->queue_total_length * sizeof(_pixelchar_renderer_char), NULL, GL_DYNAMIC_DRAW);
			result = _get_status(backend);
		}
	}

	if (functions_retrieved) backend->func.glBindBuffer(GL_ARRAY_BUFFER, 0);

	size_t pixelchar_opengl_vertex_shader_code_size = sizeof(pixelchar_opengl_vertex_shader_code) - 1;
	size_t pixelchar_opengl_fragment_shader_code_size = sizeof(pixelchar_opengl_fragment_shader_code) - 1;

	GLuint vertex_shader; 
	GLuint fragment_shader;

	if (result == PIXELCHAR_SUCCESS)
	{
		if ((vertex_shader = backend->func.glCreateShader(GL_VERTEX_SHADER)) == 0) result = PIXELCHAR_ERROR_BACKEND_API;
		else vertex_shader_created = true;
		_get_status(backend);
	}
	if (result == PIXELCHAR_SUCCESS)
	{
		if ((fragment_shader = backend->func.glCreateShader(GL_FRAGMENT_SHADER)) == 0) result = PIXELCHAR_ERROR_BACKEND_API;
		else fragment_shader_created = true;
		_get_status(backend);
	}

	uint8_t* vertex_src = vertex_shader_custom ? vertex_shader_custom : pixelchar_opengl_vertex_shader_code;
	int32_t vertex_src_length = vertex_shader_custom ? vertex_shader_custom_length : pixelchar_opengl_vertex_shader_code_size;

	uint8_t* fragment_src = fragment_shader_custom ? fragment_shader_custom : pixelchar_opengl_fragment_shader_code;
	int32_t fragment_src_length = fragment_shader_custom ? fragment_shader_custom_length : pixelchar_opengl_fragment_shader_code_size;

	if (result == PIXELCHAR_SUCCESS)
	{
		backend->func.glShaderSource(vertex_shader, 1, &vertex_src, &vertex_src_length);
		backend->func.glShaderSource(fragment_shader, 1, &fragment_src, &fragment_src_length);
		result = _get_status(backend);
	}

	if (result == PIXELCHAR_SUCCESS)
	{
		backend->func.glCompileShader(vertex_shader);

		GLint status = 0;
		backend->func.glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE) result = PIXELCHAR_ERROR_BACKEND_API;
		_get_status(backend);
	}
	if (result == PIXELCHAR_SUCCESS)
	{
		backend->func.glCompileShader(fragment_shader);

		GLint status = 0;
		backend->func.glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE) result = PIXELCHAR_ERROR_BACKEND_API;
		_get_status(backend);
	}
	if (result == PIXELCHAR_SUCCESS)
	{
		if ((backend->shader_program = backend->func.glCreateProgram()) == 0) result = PIXELCHAR_ERROR_BACKEND_API;
		else program_created = true;
		_get_status(backend);
	}
	if (result == PIXELCHAR_SUCCESS)
	{
		backend->func.glAttachShader(backend->shader_program, vertex_shader);
		backend->func.glAttachShader(backend->shader_program, fragment_shader);
		result = _get_status(backend);
	}
	if (result == PIXELCHAR_SUCCESS)
	{
		backend->func.glLinkProgram(backend->shader_program);

		GLint status = 0;
		backend->func.glGetProgramiv(backend->shader_program, GL_LINK_STATUS, &status);
		if (status == GL_FALSE) result = PIXELCHAR_ERROR_BACKEND_API;
		_get_status(backend);
	}
	if (result == PIXELCHAR_SUCCESS)
	{
		backend->uniform_location_screen_size = backend->func.glGetUniformLocation(backend->shader_program, "screen_size");
		backend->uniform_location_shadow_color_devisor = backend->func.glGetUniformLocation(backend->shader_program, "shadow_color_devisor");
		backend->uniform_location_draw_mode = backend->func.glGetUniformLocation(backend->shader_program, "draw_mode");

		if (
			backend->uniform_location_screen_size == -1 ||
			backend->uniform_location_shadow_color_devisor == -1 ||
			backend->uniform_location_draw_mode == -1
			) result = PIXELCHAR_ERROR_BACKEND_API;
		_get_status(backend);
	}

	if (result != PIXELCHAR_SUCCESS && program_created) backend->func.glDeleteProgram(backend->shader_program);
	if (fragment_shader_created) backend->func.glDeleteShader(fragment_shader);
	if (vertex_shader_created) backend->func.glDeleteShader(vertex_shader);
	if (result != PIXELCHAR_SUCCESS && vbos_created) backend->func.glDeleteBuffers(backend->resource_frame_count, backend->vbos);
	if (result != PIXELCHAR_SUCCESS && vao_created) backend->func.glDeleteVertexArrays(1, &backend->vao);
	if (result != PIXELCHAR_SUCCESS && ebo_created) backend->func.glDeleteBuffers(1, &backend->ebo);
	if (functions_retrieved) _get_status(backend);
	if (result != PIXELCHAR_SUCCESS && backend_memory_allocated) free(backend);

	if (result == PIXELCHAR_SUCCESS)
	{
		renderer->backends[backendSlotIndex].data = backend;
		renderer->backends[backendSlotIndex].deinitialize_function = pixelcharRendererBackendOpenGLDeinitialize;
		renderer->backends[backendSlotIndex].font_backend_add_reference_function = _font_backend_opengl_add_reference;
		renderer->backends[backendSlotIndex].font_backend_sub_reference_function = _font_backend_opengl_sub_reference;

		for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONT_COUNT; i++)
		{
			if (renderer->fonts[i] != NULL)
			{
				if (_font_backend_opengl_add_reference(renderer, i, backendSlotIndex) == PIXELCHAR_SUCCESS)
					renderer->font_backends_referenced[i][backendSlotIndex] = true;
			}
		}
	}

	return result;
}

void pixelcharRendererBackendOpenGLDeinitialize(PixelcharRenderer renderer, uint32_t backendSlotIndex)
{
	if (renderer == NULL) return;
	if (backendSlotIndex >= PIXELCHAR_RENDERER_MAX_BACKEND_COUNT) return;
	if (renderer->backends[backendSlotIndex].data == NULL) return;

	_renderer_backend_opengl* backend = renderer->backends[backendSlotIndex].data;

	for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONT_COUNT; i++)
	{
		if (renderer->fonts[i] != NULL && renderer->font_backends_referenced[i][backendSlotIndex] == true)
			_font_backend_opengl_sub_reference(renderer, i, backendSlotIndex);

		renderer->font_backends_referenced[i][backendSlotIndex] = false;
	}

	backend->func.glDeleteProgram(backend->shader_program);
	backend->func.glDeleteBuffers(backend->resource_frame_count, backend->vbos);
	backend->func.glDeleteVertexArrays(1, &backend->vao);
	backend->func.glDeleteBuffers(1, &backend->ebo);
	free(backend);
	
	renderer->backends[backendSlotIndex].data = NULL;
}

PixelcharResult pixelcharRendererBackendOpenGLRender(
	PixelcharRenderer renderer,
	uint32_t backendSlotIndex,
	uint32_t resourceFrameIndex,
	uint32_t width,
	uint32_t height,
	float shadowDevisorR,
	float shadowDevisorG,
	float shadowDevisorB,
	float shadowDevisorA
)
{
	if (renderer == NULL) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;
	if (backendSlotIndex >= PIXELCHAR_RENDERER_MAX_BACKEND_COUNT) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;
	if (width == 0) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;
	if (height == 0) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;
	if (renderer->queue_filled_length == 0) return PIXELCHAR_SUCCESS;
	if (renderer->backends[backendSlotIndex].data == NULL) return PIXELCHAR_ERROR_BACKEND_SLOT_NOT_IN_USED;

	_renderer_backend_opengl* backend = renderer->backends[backendSlotIndex].data;
	if (resourceFrameIndex >= backend->resource_frame_count) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;

	_pixelchar_renderer_convert_queue(renderer, backendSlotIndex);

	backend->func.glUseProgram(backend->shader_program);

	for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONT_COUNT; i++)
	{
		if (renderer->fonts[i]) if (renderer->font_backends_referenced[i][backendSlotIndex] == true) backend->func.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i, ((_font_backend_opengl*)renderer->fonts[i]->backends[backendSlotIndex])->buffer);
	}

	backend->func.glBindBuffer(GL_ARRAY_BUFFER, backend->vbos[resourceFrameIndex]);
	backend->func.glBufferSubData(GL_ARRAY_BUFFER, 0, renderer->queue_filled_length * sizeof(_pixelchar_renderer_char), renderer->queue);
	backend->func.glBindBuffer(GL_ARRAY_BUFFER, 0);

	backend->func.glBindVertexArray(backend->vao);
	backend->func.glBindVertexBuffer(0, backend->vbos[resourceFrameIndex], 0, sizeof(_pixelchar_renderer_char));

	backend->func.glUniform2i(backend->uniform_location_screen_size, width, height);
	backend->func.glUniform4f(backend->uniform_location_shadow_color_devisor, shadowDevisorR, shadowDevisorG, shadowDevisorB, shadowDevisorA);

	for (uint32_t i = 0; i < 3; i++)
	{
		backend->func.glUniform1ui(backend->uniform_location_draw_mode, i);

		backend->func.glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, renderer->queue_filled_length);
	}

	return PIXELCHAR_SUCCESS;
}