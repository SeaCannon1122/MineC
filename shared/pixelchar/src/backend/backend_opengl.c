#include <pixelchar_internal.h>
#include <pixelchar/backend/backend_opengl.h>

#include <GL/glcorearb.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "opengl_vertex_shader.h"
#include "opengl_fragment_shader.h"

#define OPENGL_FUNCTION_LIST\
	OPENGL_FUNCTION(PFNGLGENBUFFERSPROC, glGenBuffers)\
	OPENGL_FUNCTION(PFNGLBINDBUFFERPROC, glBindBuffer)\
	OPENGL_FUNCTION(PFNGLBUFFERDATAPROC, glBufferData)\
	OPENGL_FUNCTION(PFNGLBUFFERSUBDATAPROC, glBufferSubData)\
	OPENGL_FUNCTION(PFNGLDELETEBUFFERSPROC, glDeleteBuffers)\
	OPENGL_FUNCTION(PFNGLGETERRORPROC, glGetError)\
	OPENGL_FUNCTION(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays)\
	OPENGL_FUNCTION(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray)\
	OPENGL_FUNCTION(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray)\
	OPENGL_FUNCTION(PFNGLVERTEXATTRIBIFORMATPROC, glVertexAttribIFormat)\
	OPENGL_FUNCTION(PFNGLVERTEXATTRIBFORMATPROC, glVertexAttribFormat)\
	OPENGL_FUNCTION(PFNGLVERTEXATTRIBBINDINGPROC, glVertexAttribBinding)\
	OPENGL_FUNCTION(PFNGLVERTEXBINDINGDIVISORPROC, glVertexBindingDivisor)\
	OPENGL_FUNCTION(PFNGLCREATESHADERPROC, glCreateShader)\
	OPENGL_FUNCTION(PFNGLSHADERSOURCEPROC, glShaderSource)\
	OPENGL_FUNCTION(PFNGLCOMPILESHADERPROC, glCompileShader)\
	OPENGL_FUNCTION(PFNGLCREATEPROGRAMPROC, glCreateProgram)\
	OPENGL_FUNCTION(PFNGLATTACHSHADERPROC, glAttachShader)\
	OPENGL_FUNCTION(PFNGLLINKPROGRAMPROC, glLinkProgram)\
	OPENGL_FUNCTION(PFNGLGETSHADERIVPROC, glGetShaderiv)\
	OPENGL_FUNCTION(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog)\
	OPENGL_FUNCTION(PFNGLGETPROGRAMIVPROC, glGetProgramiv)\
	OPENGL_FUNCTION(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog)\
	OPENGL_FUNCTION(PFNGLDELETESHADERPROC, glDeleteShader)\
	OPENGL_FUNCTION(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation)\
	OPENGL_FUNCTION(PFNGLDELETEPROGRAMPROC, glDeleteProgram)\
	OPENGL_FUNCTION(PFNGLDELETEVERTEXARRAYSPROC, glDeleteVertexArrays)\
	OPENGL_FUNCTION(PFNGLUSEPROGRAMPROC, glUseProgram)\
	OPENGL_FUNCTION(PFNGLBINDBUFFERBASEPROC, glBindBufferBase)\
	OPENGL_FUNCTION(PFNGLBINDVERTEXBUFFERPROC, glBindVertexBuffer)\
	OPENGL_FUNCTION(PFNGLUNIFORM2IPROC, glUniform2i)\
	OPENGL_FUNCTION(PFNGLUNIFORM4FPROC, glUniform4f)\
	OPENGL_FUNCTION(PFNGLUNIFORM1UIPROC, glUniform1ui)\
	OPENGL_FUNCTION(PFNGLDRAWELEMENTSINSTANCEDPROC, glDrawElementsInstanced)\

typedef struct _font_backend_opengl
{
	GLuint buffer;
} _font_backend_opengl;

typedef struct _renderer_backend_opengl
{
	struct
	{
#define OPENGL_FUNCTION(signature, name) signature name;
		OPENGL_FUNCTION_LIST
#undef OPENGL_FUNCTION
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
		if (renderer_backend->func.glGetError() != GL_NO_ERROR)
		{
			free(font_backend);
			return PIXELCHAR_ERROR_BACKEND_API;
		}

		renderer_backend->func.glBindBuffer(GL_SHADER_STORAGE_BUFFER, font_backend->buffer);
		renderer_backend->func.glBufferData(GL_SHADER_STORAGE_BUFFER, renderer->fonts[font_index]->bitmaps_count * renderer->fonts[font_index]->resolution * renderer->fonts[font_index]->resolution / 8, renderer->fonts[font_index]->bitmaps, GL_STATIC_DRAW);
	
		if (renderer_backend->func.glGetError() != GL_NO_ERROR)
		{
			renderer_backend->func.glDeleteBuffers(1, &font_backend->buffer);
			free(font_backend);
			return PIXELCHAR_ERROR_BACKEND_API;
		}

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
	for (uint32_t i = 0; backend->func.glGetError() != GL_NO_ERROR && i < 16; i++) result = PIXELCHAR_ERROR_BACKEND_API;
	return result;
}

GLuint _compile_shader(
	_renderer_backend_opengl* backend,
	GLenum type,
	const uint8_t* source,
	size_t source_length,
	void (*log_function)(void* userParam, uint8_t* logText),
	void* user_param
)
{
	GLuint shader = backend->func.glCreateShader(type);
	if (shader == 0)
	{
		_get_status(backend);
		return 0;
	}

	backend->func.glShaderSource(shader, 1, &source, &source_length);
	backend->func.glCompileShader(shader);

	GLint success = GL_FALSE;
	backend->func.glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (success == GL_FALSE)
	{
		if (log_function)
		{
			GLint log_buffer_size = 0;
			backend->func.glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_buffer_size);
			log_buffer_size += 64;

			uint8_t* log_buffer = (uint8_t*)malloc(log_buffer_size);
			if (log_buffer)
			{
				size_t free_space_start = snprintf(log_buffer, log_buffer_size, type == GL_VERTEX_SHADER ? "Failed to compile vertex shader: " : "Failed to compile fragment shader: ");
				backend->func.glGetShaderInfoLog(shader, log_buffer_size - free_space_start, NULL, &log_buffer[free_space_start]);
				log_function(user_param, log_buffer);
				free(log_buffer);
			}
		}
		
		backend->func.glDeleteShader(shader);
		shader = 0;
	}

	_get_status(backend);
	return shader;
}

GLuint _create_program
(
	_renderer_backend_opengl* backend,
	GLuint vertex_shader, 
	GLuint fragment_shader,
	void (*log_function)(void* userParam, uint8_t* logText),
	void* user_param
)
{
	GLuint program = backend->func.glCreateProgram();
	if (program == 0)
	{
		_get_status(backend);
		return 0;
	}

	backend->func.glAttachShader(program, vertex_shader);
	backend->func.glAttachShader(program, fragment_shader);
	backend->func.glLinkProgram(program);

	GLint success = GL_FALSE;
	backend->func.glGetProgramiv(program, GL_LINK_STATUS, &success);

	if (success == GL_FALSE)
	{
		if (log_function)
		{
			GLint log_buffer_size = 0;
			backend->func.glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_buffer_size);
			log_buffer_size += 64;

			uint8_t* log_buffer = (uint8_t*)malloc(log_buffer_size);
			if (log_buffer)
			{
				size_t free_space_start = snprintf(log_buffer, log_buffer_size, "Failed to link program: ");
				backend->func.glGetProgramInfoLog(program, log_buffer_size - free_space_start, NULL, &log_buffer[free_space_start]);
				log_function(user_param, log_buffer);
				free(log_buffer);
			}
		}
		backend->func.glDeleteProgram(program);
		program = 0;
	}

	_get_status(backend);
	return program;
}

PixelcharResult pixelcharRendererBackendOpenGLInitialize(
	PixelcharRenderer renderer,
	uint32_t backendSlotIndex,
	uint32_t resourceFrameCount,
	void* (*pfnglGetProcAddress)(uint8_t*),
	uint8_t* customVertexShaderSource,
	size_t customVertexShaderSourceLength,
	uint8_t* customFragmentShaderSource,
	size_t customFragmentShaderSourceLength,
	void (*shaderErrorLogFunction)(void* userParam, uint8_t* logText),
	void* shaderErrorLogFunctionUserParam
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
		program_created = false
	;

	_renderer_backend_opengl* backend;
	if ((backend = calloc(1, sizeof(_renderer_backend_opengl))) == NULL) result = PIXELCHAR_ERROR_OUT_OF_MEMORY;
	else backend_memory_allocated = true;

	if (result == PIXELCHAR_SUCCESS)
	{
		backend->resource_frame_count = resourceFrameCount;

		struct load_entry { void** load_dst; uint8_t* func_name; };

		struct load_entry load_entries[] =
		{
#define OPENGL_FUNCTION(signature, name) {(void**)&backend->func.name, #name},
		OPENGL_FUNCTION_LIST
#undef OPENGL_FUNCTION
		};

		for (uint32_t i = 0; i < sizeof(load_entries) / sizeof(load_entries[0]); i++) if ((*load_entries[i].load_dst = (void**)pfnglGetProcAddress(load_entries[i].func_name)) == NULL)
		{
			result = PIXELCHAR_ERROR_BACKEND_API;
			break;
		}
		functions_retrieved = true;
	}

	if (functions_retrieved) result = _get_status(backend);

	if (result == PIXELCHAR_SUCCESS)
	{
		backend->func.glGenVertexArrays(1, &backend->vao);
		if ((result = _get_status(backend)) == PIXELCHAR_SUCCESS) vao_created = true;
	}
	if (result == PIXELCHAR_SUCCESS)
	{
		backend->func.glBindVertexArray(backend->vao);

		backend->func.glGenBuffers(1, &backend->ebo);
		if ((result = _get_status(backend)) == PIXELCHAR_SUCCESS) ebo_created = true;
	}
	if (result == PIXELCHAR_SUCCESS)
	{
		backend->func.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, backend->ebo);

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
		if ((result = _get_status(backend)) == PIXELCHAR_SUCCESS) vbos_created = true;
	}

	for (uint32_t i = 0; i < backend->resource_frame_count && result == PIXELCHAR_SUCCESS; i++)
	{
		backend->func.glBindBuffer(GL_ARRAY_BUFFER, backend->vbos[i]);
		backend->func.glBufferData(GL_ARRAY_BUFFER, renderer->queue_total_length * sizeof(_pixelchar_renderer_char), NULL, GL_DYNAMIC_DRAW);
		result = _get_status(backend);
	}

	if (functions_retrieved) backend->func.glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (result == PIXELCHAR_SUCCESS)
	{
		GLuint vertex_shader, fragment_shader;

		if ((vertex_shader = _compile_shader(
			backend,
			GL_VERTEX_SHADER,
			customVertexShaderSource ? customVertexShaderSource : vertex_shader_source,
			customVertexShaderSource ? customVertexShaderSourceLength : sizeof(vertex_shader_source) - 1,
			shaderErrorLogFunction,
			shaderErrorLogFunctionUserParam
		)) != 0)
		{
			if ((fragment_shader = _compile_shader(
				backend,
				GL_FRAGMENT_SHADER,
				customFragmentShaderSource ? customFragmentShaderSource : fragment_shader_source,
				customFragmentShaderSource ? customFragmentShaderSourceLength : sizeof(fragment_shader_source) - 1,
				shaderErrorLogFunction,
				shaderErrorLogFunctionUserParam
			)) != 0)
			{
				backend->shader_program = _create_program(
					backend,
					vertex_shader,
					fragment_shader,
					shaderErrorLogFunction,
					shaderErrorLogFunctionUserParam
				);
				backend->func.glDeleteShader(fragment_shader);
			}
			backend->func.glDeleteShader(vertex_shader);
		}

		if (backend->shader_program == 0) result = PIXELCHAR_ERROR_BACKEND_API;
		else program_created = true;
	}

	if (result == PIXELCHAR_SUCCESS)
	{
		if ((backend->uniform_location_screen_size = backend->func.glGetUniformLocation(backend->shader_program, "screen_size")) == -1) shaderErrorLogFunction(shaderErrorLogFunctionUserParam, "Failed to get uniform location of 'screen_size' (2i)");
		if ((backend->uniform_location_shadow_color_devisor = backend->func.glGetUniformLocation(backend->shader_program, "shadow_color_devisor")) == -1) shaderErrorLogFunction(shaderErrorLogFunctionUserParam, "Failed to get uniform location of 'shadow_color_devisor' (4f)");
		if ((backend->uniform_location_draw_mode = backend->func.glGetUniformLocation(backend->shader_program, "draw_mode")) == -1) shaderErrorLogFunction(shaderErrorLogFunctionUserParam, "Failed to get uniform location of 'draw_mode' (1ui)");

		if (
			backend->uniform_location_screen_size == -1 ||
			backend->uniform_location_shadow_color_devisor == -1 ||
			backend->uniform_location_draw_mode == -1
		)
		{
			result = PIXELCHAR_ERROR_BACKEND_API;
			_get_status(backend);
		}
	}

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
	else
	{
		if (program_created) backend->func.glDeleteProgram(backend->shader_program);
		if (vbos_created) backend->func.glDeleteBuffers(backend->resource_frame_count, backend->vbos);
		if (vao_created) backend->func.glDeleteVertexArrays(1, &backend->vao);
		if (ebo_created) backend->func.glDeleteBuffers(1, &backend->ebo);
		if (backend_memory_allocated) free(backend);
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

void pixelcharRendererBackendOpenGLRender(
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
	if (renderer == NULL) return;
	if (backendSlotIndex >= PIXELCHAR_RENDERER_MAX_BACKEND_COUNT) return;
	if (width == 0) return;
	if (height == 0) return;
	if (renderer->queue_filled_length == 0) return;
	if (renderer->backends[backendSlotIndex].data == NULL) return;

	_renderer_backend_opengl* backend = renderer->backends[backendSlotIndex].data;
	if (resourceFrameIndex >= backend->resource_frame_count) return;

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
}