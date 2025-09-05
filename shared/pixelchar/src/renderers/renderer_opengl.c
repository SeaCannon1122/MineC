#include <pixelchar_internal.h>
#include <pixelchar/renderers/renderer_opengl.h>

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

typedef struct PixelcharRendererOpenGL_T
{
	struct
	{
#define OPENGL_FUNCTION(signature, name) signature name;
		OPENGL_FUNCTION_LIST
#undef OPENGL_FUNCTION
	} func;

	PixelcharFont fonts[PIXELCHAR_RENDERER_MAX_FONT_COUNT];
	GLuint opengl_fonts[PIXELCHAR_RENDERER_MAX_FONT_COUNT];
	
	GLuint vbo;
	GLuint ebo;
	GLuint vao;

	GLuint shader_program;

	GLint uniform_location_screen_size;
	GLint uniform_location_shadow_color_devisor;
	GLint uniform_location_draw_mode;

	uint32_t resource_frame_count;
	uint32_t buffer_length;
	uint32_t buffer_mext_free_indices[PIXELCHAR_RENDERER_OPENGL_MAX_RESOURCE_FRAME_COUNT];

	size_t rendering_size;
	size_t rendering_offset;
} PixelcharRendererOpenGL_T;

bool _get_status(PixelcharRendererOpenGL_T* renderer)
{
	bool success = true;
	for (uint32_t i = 0; renderer->func.glGetError() != GL_NO_ERROR && i < 16; i++) success = false;
	return success;
}

bool PixelcharRendererOpenGLUseFont(PixelcharRendererOpenGL renderer, PixelcharFont font, uint32_t fontIndex)
{
	if (renderer->fonts[fontIndex] != NULL)
	{
		renderer->fonts[fontIndex]->reference_count--;
		if (renderer->fonts[fontIndex]->destroyed && renderer->fonts[fontIndex]->reference_count == 0) free(renderer->fonts[fontIndex]);
		renderer->func.glDeleteBuffers(1, &renderer->opengl_fonts[fontIndex]);
	}
	renderer->fonts[fontIndex] = NULL;
	if (font != NULL)
	{
		renderer->func.glGenBuffers(1, &renderer->opengl_fonts[fontIndex]);
		if (_get_status(renderer) == false) return false;

		renderer->func.glBindBuffer(GL_SHADER_STORAGE_BUFFER, renderer->opengl_fonts[fontIndex]);
		renderer->func.glBufferData(GL_SHADER_STORAGE_BUFFER, font->bitmaps_count * font->resolution * font->resolution / 8, font->bitmaps, GL_STATIC_DRAW);
		renderer->func.glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		if (_get_status(renderer) == false)
		{
			renderer->func.glDeleteBuffers(1, &renderer->opengl_fonts[fontIndex]);
			return false;
		}

		renderer->fonts[fontIndex] = font;
		renderer->fonts[fontIndex]->reference_count++;
	}
	return true;
}

GLuint _compile_shader(
	PixelcharRendererOpenGL_T* renderer,
	GLenum type,
	const uint8_t* source,
	size_t source_length,
	void (*log_function)(void* userParam, uint8_t* logText),
	void* user_param
)
{
	GLuint shader = renderer->func.glCreateShader(type);
	if (shader == 0)
	{
		_get_status(renderer);
		return 0;
	}

	renderer->func.glShaderSource(shader, 1, &source, &source_length);
	renderer->func.glCompileShader(shader);

	GLint success = GL_FALSE;
	renderer->func.glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (success == GL_FALSE)
	{
		if (log_function)
		{
			GLint log_buffer_size = 0;
			renderer->func.glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_buffer_size);
			log_buffer_size += 64;

			uint8_t* log_buffer = (uint8_t*)malloc(log_buffer_size);
			if (log_buffer)
			{
				size_t free_space_start = snprintf(log_buffer, log_buffer_size, type == GL_VERTEX_SHADER ? "Failed to compile vertex shader: " : "Failed to compile fragment shader: ");
				renderer->func.glGetShaderInfoLog(shader, log_buffer_size - free_space_start, NULL, &log_buffer[free_space_start]);
				log_function(user_param, log_buffer);
				free(log_buffer);
			}
		}
		
		renderer->func.glDeleteShader(shader);
		shader = 0;
	}

	_get_status(renderer);
	return shader;
}

GLuint _create_program
(
	PixelcharRendererOpenGL_T* renderer,
	GLuint vertex_shader, 
	GLuint fragment_shader,
	void (*log_function)(void* userParam, uint8_t* logText),
	void* user_param
)
{
	GLuint program = renderer->func.glCreateProgram();
	if (program == 0)
	{
		_get_status(renderer);
		return 0;
	}

	renderer->func.glAttachShader(program, vertex_shader);
	renderer->func.glAttachShader(program, fragment_shader);
	renderer->func.glLinkProgram(program);

	GLint success = GL_FALSE;
	renderer->func.glGetProgramiv(program, GL_LINK_STATUS, &success);

	if (success == GL_FALSE)
	{
		if (log_function)
		{
			GLint log_buffer_size = 0;
			renderer->func.glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_buffer_size);
			log_buffer_size += 64;

			uint8_t* log_buffer = (uint8_t*)malloc(log_buffer_size);
			if (log_buffer)
			{
				size_t free_space_start = snprintf(log_buffer, log_buffer_size, "Failed to link program: ");
				renderer->func.glGetProgramInfoLog(program, log_buffer_size - free_space_start, NULL, &log_buffer[free_space_start]);
				log_function(user_param, log_buffer);
				free(log_buffer);
			}
		}
		renderer->func.glDeleteProgram(program);
		program = 0;
	}

	_get_status(renderer);
	return program;
}

bool PixelcharRendererOpenGLCreate(
	void* (*pfnglGetProcAddress)(void* userParam, uint8_t*),
	void* pfnglGetProcAddressUserParam,
	uint8_t* customVertexShaderSource,
	size_t customVertexShaderSourceLength,
	uint8_t* customFragmentShaderSource,
	size_t customFragmentShaderSourceLength,
	void (*shaderErrorLogFunction)(void* userParam, uint8_t* logText),
	void* shaderErrorLogFunctionUserParam,
	uint32_t resourceFrameCount,
	uint32_t maxResourceFrameCharacterCount,
	PixelcharRendererOpenGL* pRenderer
)
{
	bool success = true;

	bool
		vao_created = false,
		ebo_created = false,
		vbo_created = false,
		program_created = false
	;

	PixelcharRendererOpenGL_T* renderer;
	if ((renderer = calloc(1, sizeof(PixelcharRendererOpenGL_T))) == NULL) return false;

	renderer->buffer_length = maxResourceFrameCharacterCount;
	renderer->resource_frame_count = resourceFrameCount;

	struct load_entry { void** load_dst; uint8_t* func_name; };

	struct load_entry load_entries[] =
	{
#define OPENGL_FUNCTION(signature, name) {(void**)&renderer->func.name, #name},
		OPENGL_FUNCTION_LIST
#undef OPENGL_FUNCTION
	};

	for (uint32_t i = 0; i < sizeof(load_entries) / sizeof(load_entries[0]); i++) if ((*load_entries[i].load_dst = (void**)pfnglGetProcAddress(pfnglGetProcAddressUserParam, load_entries[i].func_name)) == NULL)
	{
		success = false;
		break;
	}

	if (success)
	{
		renderer->func.glGenVertexArrays(1, &renderer->vao);
		if (success = _get_status(renderer)) vao_created = true;
	}
	if (success)
	{
		renderer->func.glBindVertexArray(renderer->vao);

		renderer->func.glGenBuffers(1, &renderer->ebo);
		if (success = _get_status(renderer)) ebo_created = true;
	}
	if (success)
	{
		renderer->func.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ebo);

		GLuint indices[] = { 0, 1, 2, 1, 3, 2 };
		renderer->func.glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		renderer->func.glEnableVertexAttribArray(0);
		renderer->func.glVertexAttribIFormat(0, 1, GL_UNSIGNED_INT, 0);
		renderer->func.glVertexAttribBinding(0, 0);
		renderer->func.glVertexBindingDivisor(0, 1);

		renderer->func.glEnableVertexAttribArray(1);
		renderer->func.glVertexAttribIFormat(1, 1, GL_UNSIGNED_INT, offsetof(_pixelchar_renderer_char, flags));
		renderer->func.glVertexAttribBinding(1, 0);
		renderer->func.glVertexBindingDivisor(1, 1);

		renderer->func.glEnableVertexAttribArray(2);
		renderer->func.glVertexAttribIFormat(2, 1, GL_UNSIGNED_SHORT, offsetof(_pixelchar_renderer_char, fontIndex));
		renderer->func.glVertexAttribBinding(2, 0);
		renderer->func.glVertexBindingDivisor(2, 1);

		renderer->func.glEnableVertexAttribArray(3);
		renderer->func.glVertexAttribIFormat(3, 1, GL_UNSIGNED_SHORT, offsetof(_pixelchar_renderer_char, fontResolution));
		renderer->func.glVertexAttribBinding(3, 0);
		renderer->func.glVertexBindingDivisor(3, 1);

		renderer->func.glEnableVertexAttribArray(4);
		renderer->func.glVertexAttribIFormat(4, 1, GL_UNSIGNED_SHORT, offsetof(_pixelchar_renderer_char, scale));
		renderer->func.glVertexAttribBinding(4, 0);
		renderer->func.glVertexBindingDivisor(4, 1);

		renderer->func.glEnableVertexAttribArray(5);
		renderer->func.glVertexAttribIFormat(5, 1, GL_UNSIGNED_BYTE, offsetof(_pixelchar_renderer_char, bitmapWidth));
		renderer->func.glVertexAttribBinding(5, 0);
		renderer->func.glVertexBindingDivisor(5, 1);

		renderer->func.glEnableVertexAttribArray(6);
		renderer->func.glVertexAttribIFormat(6, 2, GL_INT, offsetof(_pixelchar_renderer_char, position));
		renderer->func.glVertexAttribBinding(6, 0);
		renderer->func.glVertexBindingDivisor(6, 1);

		renderer->func.glEnableVertexAttribArray(7);
		renderer->func.glVertexAttribFormat(7, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(_pixelchar_renderer_char, color));
		renderer->func.glVertexAttribBinding(7, 0);
		renderer->func.glVertexBindingDivisor(7, 1);

		renderer->func.glEnableVertexAttribArray(8);
		renderer->func.glVertexAttribFormat(8, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(_pixelchar_renderer_char, backgroundColor));
		renderer->func.glVertexAttribBinding(8, 0);
		renderer->func.glVertexBindingDivisor(8, 1);

		success = _get_status(renderer);
	}

	if (vao_created) renderer->func.glBindVertexArray(0);

	if (success)
	{
		renderer->func.glGenBuffers(1, &renderer->vbo);
		if (success = _get_status(renderer)) vbo_created = true;
	}

	if (success)
	{
		renderer->func.glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
		renderer->func.glBufferData(GL_ARRAY_BUFFER, renderer->buffer_length * sizeof(Pixelchar) * renderer->resource_frame_count, NULL, GL_DYNAMIC_DRAW);
		success = _get_status(renderer);
	}

	if (vbo_created) renderer->func.glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (success)
	{
		GLuint vertex_shader, fragment_shader;

		if ((vertex_shader = _compile_shader(
			renderer,
			GL_VERTEX_SHADER,
			customVertexShaderSource ? customVertexShaderSource : vertex_shader_source,
			customVertexShaderSource ? customVertexShaderSourceLength : sizeof(vertex_shader_source) - 1,
			shaderErrorLogFunction,
			shaderErrorLogFunctionUserParam
		)) != 0)
		{
			if ((fragment_shader = _compile_shader(
				renderer,
				GL_FRAGMENT_SHADER,
				customFragmentShaderSource ? customFragmentShaderSource : fragment_shader_source,
				customFragmentShaderSource ? customFragmentShaderSourceLength : sizeof(fragment_shader_source) - 1,
				shaderErrorLogFunction,
				shaderErrorLogFunctionUserParam
			)) != 0)
			{
				renderer->shader_program = _create_program(
					renderer,
					vertex_shader,
					fragment_shader,
					shaderErrorLogFunction,
					shaderErrorLogFunctionUserParam
				);
				renderer->func.glDeleteShader(fragment_shader);
			}
			renderer->func.glDeleteShader(vertex_shader);
		}

		if (renderer->shader_program == 0) success = false;
		else program_created = true;
	}

	if (success)
	{
		if ((renderer->uniform_location_screen_size = renderer->func.glGetUniformLocation(renderer->shader_program, "screen_size")) == -1) shaderErrorLogFunction(shaderErrorLogFunctionUserParam, "Failed to get uniform location of 'screen_size' (2i)");
		if ((renderer->uniform_location_shadow_color_devisor = renderer->func.glGetUniformLocation(renderer->shader_program, "shadow_color_devisor")) == -1) shaderErrorLogFunction(shaderErrorLogFunctionUserParam, "Failed to get uniform location of 'shadow_color_devisor' (4f)");
		if ((renderer->uniform_location_draw_mode = renderer->func.glGetUniformLocation(renderer->shader_program, "draw_mode")) == -1) shaderErrorLogFunction(shaderErrorLogFunctionUserParam, "Failed to get uniform location of 'draw_mode' (1ui)");

		if (
			renderer->uniform_location_screen_size == -1 ||
			renderer->uniform_location_shadow_color_devisor == -1 ||
			renderer->uniform_location_draw_mode == -1
		) success = false;
	}
	if (success == false)
	{
		if (program_created) renderer->func.glDeleteProgram(renderer->shader_program);
		if (vbo_created) renderer->func.glDeleteBuffers(1, renderer->vbo);
		if (vao_created) renderer->func.glDeleteVertexArrays(1, &renderer->vao);
		if (ebo_created) renderer->func.glDeleteBuffers(1, &renderer->ebo);
		free(renderer);
	}
	else *pRenderer = renderer;

	_get_status(renderer);
	return success;
}

void PixelcharRendererOpenGLDestroy(PixelcharRendererOpenGL renderer)
{
	for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONT_COUNT; i++) if (renderer->fonts[i] != NULL)
	{
		renderer->func.glDeleteBuffers(1, &renderer->opengl_fonts[i]);
		renderer->fonts[i]->reference_count--;
		if (renderer->fonts[i]->destroyed && renderer->fonts[i]->reference_count == 0) free(renderer->fonts[i]);
	}

	renderer->func.glDeleteProgram(renderer->shader_program);
	renderer->func.glDeleteBuffers(1, &renderer->vbo);
	renderer->func.glDeleteVertexArrays(1, &renderer->vao);
	renderer->func.glDeleteBuffers(1, &renderer->ebo);
	free(renderer);
}

void PixelcharRendererVulkanUpdateRenderingData(
	PixelcharRendererOpenGL renderer,
	Pixelchar* pCharacters,
	uint32_t characterCount,
	uint32_t resourceFrameIndex
)
{
	uint32_t char_copy_count = (
		characterCount <= renderer->buffer_length - renderer->buffer_mext_free_indices[resourceFrameIndex] ?
		characterCount :
		renderer->buffer_length - renderer->buffer_mext_free_indices[resourceFrameIndex]
	);

	if (char_copy_count == 0) return;
		
	_convert_chars_to_render_chars(pCharacters, char_copy_count, renderer->fonts);

	renderer->rendering_offset = (resourceFrameIndex * renderer->buffer_length + renderer->buffer_mext_free_indices[resourceFrameIndex]) * sizeof(Pixelchar);
	renderer->rendering_size = sizeof(Pixelchar) * char_copy_count;
	renderer->buffer_mext_free_indices[resourceFrameIndex] += char_copy_count;

	renderer->func.glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
	renderer->func.glBufferSubData(GL_ARRAY_BUFFER, renderer->rendering_offset, renderer->rendering_size, pCharacters);
	renderer->func.glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}

void PixelcharRendererVulkanResetResourceFrame(PixelcharRendererOpenGL renderer, uint32_t resourceFrameIndex)
{
	renderer->buffer_mext_free_indices[resourceFrameIndex] = 0;
}

void PixelcharRendererVulkanRender(
	PixelcharRendererOpenGL renderer,
	uint32_t width,
	uint32_t height,
	float shadowDevisorR,
	float shadowDevisorG,
	float shadowDevisorB,
	float shadowDevisorA
)
{
	renderer->func.glUseProgram(renderer->shader_program);

	for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONT_COUNT; i++)
		if (renderer->fonts[i]) renderer->func.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i, renderer->opengl_fonts[i]);

	renderer->func.glBindVertexArray(renderer->vao);
	renderer->func.glBindVertexBuffer(0, renderer->vbo, renderer->rendering_offset, sizeof(Pixelchar));

	renderer->func.glUniform2i(renderer->uniform_location_screen_size, width, height);
	renderer->func.glUniform4f(renderer->uniform_location_shadow_color_devisor, shadowDevisorR, shadowDevisorG, shadowDevisorB, shadowDevisorA);

	for (uint32_t i = 0; i < 3; i++)
	{
		renderer->func.glUniform1ui(renderer->uniform_location_draw_mode, i);

		renderer->func.glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, renderer->rendering_size / sizeof(Pixelchar));
	}
}