#include <pixelchar/impl/impl_opengl.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "opengl_vertex_shader.h"
#include "opengl_fragment_shader.h"

GLuint _compile_shader(
	PixelcharImplOpenGLRenderer* renderer,
	GLenum type,
	const uint8_t* source,
	size_t source_length,
	PixelcharLogCallback logCallback,
	void* logCallbackUserParam
)
{
	GLuint shader = renderer->pFactory->func.glCreateShader(type);

	renderer->pFactory->func.glShaderSource(shader, 1, &source, &source_length);
	renderer->pFactory->func.glCompileShader(shader);

	GLint success = GL_FALSE;
	renderer->pFactory->func.glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (success == GL_FALSE)
	{
		if (logCallback)
		{
			GLint log_buffer_size = 0;
			renderer->pFactory->func.glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_buffer_size);
			log_buffer_size += 64;

			uint8_t* log_buffer = (uint8_t*)malloc(log_buffer_size);
			if (log_buffer)
			{
				size_t free_space_start = snprintf(log_buffer, log_buffer_size, type == GL_VERTEX_SHADER ? "ERROR: Failed to compile vertex shader: " : "Failed to compile fragment shader: ");
				renderer->pFactory->func.glGetShaderInfoLog(shader, log_buffer_size - free_space_start, NULL, &log_buffer[free_space_start]);
				logCallback(log_buffer, logCallbackUserParam);
				free(log_buffer);
			}
		}
		
		renderer->pFactory->func.glDeleteShader(shader);
		shader = 0;
	}
	return shader;
}

GLuint _create_program
(
	PixelcharImplOpenGLRenderer* renderer,
	GLuint vertex_shader, 
	GLuint fragment_shader,
	PixelcharLogCallback logCallback,
	void* logCallbackUserParam
)
{
	GLuint program = renderer->pFactory->func.glCreateProgram();

	renderer->pFactory->func.glAttachShader(program, vertex_shader);
	renderer->pFactory->func.glAttachShader(program, fragment_shader);
	renderer->pFactory->func.glLinkProgram(program);

	GLint success = GL_FALSE;
	renderer->pFactory->func.glGetProgramiv(program, GL_LINK_STATUS, &success);

	if (success == GL_FALSE)
	{
		if (logCallback)
		{
			GLint log_buffer_size = 0;
			renderer->pFactory->func.glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_buffer_size);
			log_buffer_size += 64;

			uint8_t* log_buffer = (uint8_t*)malloc(log_buffer_size);
			if (log_buffer)
			{
				size_t free_space_start = snprintf(log_buffer, log_buffer_size, "Failed to link program: ");
				renderer->pFactory->func.glGetProgramInfoLog(program, log_buffer_size - free_space_start, NULL, &log_buffer[free_space_start]);
				logCallback(log_buffer, logCallbackUserParam);
				free(log_buffer);
			}
		}
		renderer->pFactory->func.glDeleteProgram(program);
		program = 0;
	}
	return program;
}

bool PixelcharImplOpenGLFactoryCreate(
	void* (*pfnglGetProcAddress)(void* userParam, uint8_t* name),
	void* pfnglGetProcAddressUserParam,
	PixelcharImplOpenGLFactory* pFactory,
	PixelcharLogCallback logCallback,
	void* logCallbackUserParam
)
{
	struct load_entry { void** load_dst; uint8_t* func_name; };

	struct load_entry load_entries[] =
	{
#define PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(signature, name) {(void**)&pFactory->func.name, #name},
		PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION_LIST
#undef PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION
	};

	for (uint32_t i = 0; i < sizeof(load_entries) / sizeof(load_entries[0]); i++)
		if ((*load_entries[i].load_dst = (void**)pfnglGetProcAddress(pfnglGetProcAddressUserParam, load_entries[i].func_name)) == NULL)
		{
			if (logCallback)
			{
				uint8_t message_buffer[1024];
				snprintf(message_buffer, sizeof(message_buffer), "ERROR: Could not load function '%s'", load_entries[i].func_name);
				logCallback(message_buffer, logCallbackUserParam);
			}
			return false;
		}

	pFactory->func.glGenVertexArrays(1, &pFactory->vertexArray);
	pFactory->func.glBindVertexArray(pFactory->vertexArray);

	pFactory->func.glGenBuffers(1, &pFactory->elementBuffer);
	pFactory->func.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pFactory->elementBuffer);

	GLuint indices[] = { 0, 1, 2, 1, 3, 2 };
	pFactory->func.glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	pFactory->func.glEnableVertexAttribArray(0);
	pFactory->func.glVertexAttribIFormat(0, 4, GL_INT, 0);
	pFactory->func.glVertexAttribBinding(0, 0);
	pFactory->func.glVertexBindingDivisor(0, 1);

	pFactory->func.glEnableVertexAttribArray(1);
	pFactory->func.glVertexAttribIFormat(1, 4, GL_INT, offsetof(Pixelchar, bitmapIndex));
	pFactory->func.glVertexAttribBinding(1, 0);
	pFactory->func.glVertexBindingDivisor(1, 1);

	pFactory->func.glEnableVertexAttribArray(2);
	pFactory->func.glVertexAttribFormat(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(Pixelchar, color));
	pFactory->func.glVertexAttribBinding(2, 0);
	pFactory->func.glVertexBindingDivisor(2, 1);

	pFactory->func.glEnableVertexAttribArray(3);
	pFactory->func.glVertexAttribFormat(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(Pixelchar, backgroundColor));
	pFactory->func.glVertexAttribBinding(3, 0);
	pFactory->func.glVertexBindingDivisor(3, 1);

	pFactory->func.glBindVertexArray(0);

	return true;
}

void PixelcharImplOpenGLFactoryDestroy(PixelcharImplOpenGLFactory* pFactory)
{
	pFactory->func.glDeleteVertexArrays(1, &pFactory->vertexArray);
	pFactory->func.glDeleteBuffers(1, &pFactory->elementBuffer);
}

bool PixelcharImplOpenGLRendererCreate(
	PixelcharImplOpenGLFactory* pFactory,
	const uint8_t* pCustomVertexShaderSource,
	size_t customVertexShaderSourceLength,
	const uint8_t* pCustomFragmentShaderSource,
	size_t customFragmentShaderSourceLength,
	PixelcharImplOpenGLRenderer* pRenderer,
	PixelcharLogCallback logCallback,
	void* logCallbackUserParam
)
{
	bool success = true;
	pRenderer->pFactory = pFactory;

	GLuint vertex_shader, fragment_shader;
	pRenderer->shaderProgram = 0;

	if ((vertex_shader = _compile_shader(
		pRenderer,
		GL_VERTEX_SHADER,
		pCustomVertexShaderSource ? pCustomVertexShaderSource : vertex_shader_source,
		pCustomVertexShaderSource ? customVertexShaderSourceLength : sizeof(vertex_shader_source) - 1,
		logCallback,
		logCallbackUserParam
	)) != 0)
	{
		if ((fragment_shader = _compile_shader(
			pRenderer,
			GL_FRAGMENT_SHADER,
			pCustomFragmentShaderSource ? pCustomFragmentShaderSource : fragment_shader_source,
			pCustomFragmentShaderSource ? customFragmentShaderSourceLength : sizeof(fragment_shader_source) - 1,
			logCallback,
			logCallbackUserParam
		)) != 0)
		{
			pRenderer->shaderProgram = _create_program(
				pRenderer,
				vertex_shader,
				fragment_shader,
				logCallback,
				logCallbackUserParam
			);
			pRenderer->pFactory->func.glDeleteShader(fragment_shader);
		}
		pRenderer->pFactory->func.glDeleteShader(vertex_shader);
	}

	if (pRenderer->shaderProgram == 0) success = false;

	if (success)
	{
		if ((pRenderer->uniformLocationFontResolution = pRenderer->pFactory->func.glGetUniformLocation(pRenderer->shaderProgram, "font_resolution")) == -1)
		{
			logCallback("Failed to get uniform location of 'font_resolution'", logCallbackUserParam);
			success = false;
		}
		if ((pRenderer->uniformLocationBitmapCount = pRenderer->pFactory->func.glGetUniformLocation(pRenderer->shaderProgram, "bitmap_count")) == -1)
		{ 
			logCallback("Failed to get uniform location of 'bitmap_count'", logCallbackUserParam); 
			success = false; 
		}
		if ((pRenderer->uniformLocationScreenToNDC = pRenderer->pFactory->func.glGetUniformLocation(pRenderer->shaderProgram, "screen_to_ndc")) == -1)
		{
			logCallback("Failed to get uniform location of 'screen_to_ndc'", logCallbackUserParam); 
			success = false; 
		}
		if ((pRenderer->uniformLocationShadowColorDevisor = pRenderer->pFactory->func.glGetUniformLocation(pRenderer->shaderProgram, "shadow_color_devisor")) == -1)
		{ 
			logCallback("Failed to get uniform location of 'shadow_color_devisor'", logCallbackUserParam); 
			success = false; 
		}
		if ((pRenderer->uniformLocationDrawMode = pRenderer->pFactory->func.glGetUniformLocation(pRenderer->shaderProgram, "draw_mode")) == -1)
		{ 
			logCallback("Failed to get uniform location of 'draw_mode'", logCallbackUserParam); 
			success = false; 
		}
	}

	if (success == false) pRenderer->pFactory->func.glDeleteProgram(pRenderer->shaderProgram);
	return success;
}

void PixelcharImplOpenGLRendererDestroy(PixelcharImplOpenGLRenderer* pRenderer)
{
	pRenderer->pFactory->func.glDeleteProgram(pRenderer->shaderProgram);
}

bool PixelcharImplOpenGLFontCreate(
	PixelcharImplOpenGLFactory* pFactory,
	PixelcharFont* pSourceFont,
	PixelcharImplOpenGLFont* pFont,
	PixelcharLogCallback logCallback,
	void* logCallbackUserParam
)
{
	pFont->pFactory = pFactory;
	pFont->resolution = pSourceFont->resolution;
	pFont->bitmapCount = pSourceFont->bitmapCount;

	pFont->pFactory->func.glGenBuffers(1, &pFont->ssbo);
	pFont->pFactory->func.glBindBuffer(GL_SHADER_STORAGE_BUFFER, pFont->ssbo);
	pFont->pFactory->func.glBufferData(GL_SHADER_STORAGE_BUFFER, pSourceFont->bitmapCount * pSourceFont->resolution * pSourceFont->resolution / 8, pSourceFont->pBitmaps, GL_STATIC_DRAW);
	pFont->pFactory->func.glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	return true;
}

void PixelcharImplOpenGLFontDestroy(PixelcharImplOpenGLFont* pFont)
{
	pFont->pFactory->func.glDeleteBuffers(1, &pFont->ssbo);
}

void PixelcharImplOpenGLRender(
	PixelcharImplOpenGLRenderer* pRenderer,
	uint32_t characterCount,
	GLuint vertexBuffer,
	uint32_t vertexBufferOffset,
	PixelcharImplOpenGLFont* pFont,
	uint32_t width,
	uint32_t height,
	float shadowDevisorR,
	float shadowDevisorG,
	float shadowDevisorB,
	float shadowDevisorA
)
{
	pRenderer->pFactory->func.glUseProgram(pRenderer->shaderProgram);

	pRenderer->pFactory->func.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pFont->ssbo);
	pRenderer->pFactory->func.glUniform1i(pRenderer->uniformLocationBitmapCount, pFont->bitmapCount);
	pRenderer->pFactory->func.glUniform1i(pRenderer->uniformLocationFontResolution, pFont->resolution);

	pRenderer->pFactory->func.glBindVertexArray(pRenderer->pFactory->vertexArray);
	pRenderer->pFactory->func.glBindVertexBuffer(0, vertexBuffer, vertexBufferOffset, sizeof(Pixelchar));

	GLfloat screen_to_ndc[16] = 
	{
		2.f / width,	0.f,				0.f,	0.f,
		0.f,			-2.f / height,		0.f,	0.f,
		0.f,			0.f,				1.f,	0.f,
		-1.f,			1.f,				0.f,	1.f
	};
	pRenderer->pFactory->func.glUniformMatrix4fv(pRenderer->uniformLocationScreenToNDC, 1, GL_FALSE, screen_to_ndc);
	pRenderer->pFactory->func.glUniform4f(pRenderer->uniformLocationShadowColorDevisor, shadowDevisorR, shadowDevisorG, shadowDevisorB, shadowDevisorA);
	

	for (uint32_t i = 0; i < 3; i++)
	{
		pRenderer->pFactory->func.glUniform1i(pRenderer->uniformLocationDrawMode, i);
		pRenderer->pFactory->func.glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, characterCount);
	}
}