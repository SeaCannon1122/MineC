#pragma once

#ifndef PIXELCHAR_IMPL_OPENGL_H
#define PIXELCHAR_IMPL_OPENGL_H

#include <pixelchar/pixelchar.h>
#include <gl/glcorearb.h>

#define PIXELCHAR_IMPL_OPENGL_MAX_RESOURCE_FRAME_COUNT 4

#define PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION_LIST\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLGENBUFFERSPROC, glGenBuffers)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLBINDBUFFERPROC, glBindBuffer)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLBUFFERDATAPROC, glBufferData)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLDELETEBUFFERSPROC, glDeleteBuffers)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLGETERRORPROC, glGetError)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLVERTEXATTRIBIFORMATPROC, glVertexAttribIFormat)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLVERTEXATTRIBFORMATPROC, glVertexAttribFormat)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLVERTEXATTRIBBINDINGPROC, glVertexAttribBinding)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLVERTEXBINDINGDIVISORPROC, glVertexBindingDivisor)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLCREATESHADERPROC, glCreateShader)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLSHADERSOURCEPROC, glShaderSource)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLCOMPILESHADERPROC, glCompileShader)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLCREATEPROGRAMPROC, glCreateProgram)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLATTACHSHADERPROC, glAttachShader)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLLINKPROGRAMPROC, glLinkProgram)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLGETSHADERIVPROC, glGetShaderiv)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLGETPROGRAMIVPROC, glGetProgramiv)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLDELETESHADERPROC, glDeleteShader)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLDELETEPROGRAMPROC, glDeleteProgram)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLDELETEVERTEXARRAYSPROC, glDeleteVertexArrays)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLUSEPROGRAMPROC, glUseProgram)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLBINDBUFFERBASEPROC, glBindBufferBase)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLBINDVERTEXBUFFERPROC, glBindVertexBuffer)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLUNIFORM4FPROC, glUniform4f)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLUNIFORM1IPROC, glUniform1i)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLUNIFORMMATRIX4FVPROC, glUniformMatrix4fv)\
	PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(PFNGLDRAWELEMENTSINSTANCEDPROC, glDrawElementsInstanced)\

typedef struct PixelcharImplOpenGLFactory
{
	struct
	{
#define PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION(signature, name) signature name;
		PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION_LIST
#undef PIXELCHAR_IMPL_OPENGL_INTERNAL_FUNCTION
	} func;

	GLuint elementBuffer;
	GLuint vertexArray;
} PixelcharImplOpenGLFactory;

typedef struct PixelcharImplOpenGLRenderer
{
	PixelcharImplOpenGLFactory* pFactory;

	GLuint shaderProgram;

	GLint uniformLocationFontResolution;
	GLint uniformLocationBitmapCount;
	GLint uniformLocationScreenToNDC;
	GLint uniformLocationShadowColorDevisor;
	GLint uniformLocationDrawMode;
} PixelcharImplOpenGLRenderer;

typedef struct PixelcharImplOpenGLFont
{
	PixelcharImplOpenGLFactory* pFactory;

	uint32_t bitmapCount;
	uint32_t resolution;

	GLuint ssbo;
} PixelcharImplOpenGLFont;

bool PixelcharImplOpenGLFactoryCreate(
	void* (*pfnglGetProcAddress)(void* userParam, uint8_t* name),
	void* pfnglGetProcAddressUserParam,
	PixelcharImplOpenGLFactory* pFactory,
	PixelcharLogCallback logCallback,
	void* logCallbackUserParam
);

void PixelcharImplOpenGLFactoryDestroy(PixelcharImplOpenGLFactory* pFactory);

bool PixelcharImplOpenGLRendererCreate(
	PixelcharImplOpenGLFactory* pFactory,
	const uint8_t* pCustomVertexShaderSource,
	size_t customVertexShaderSourceLength,
	const uint8_t* pCustomFragmentShaderSource,
	size_t customFragmentShaderSourceLength,
	PixelcharImplOpenGLRenderer* pRenderer,
	PixelcharLogCallback logCallback,
	void* logCallbackUserParam
);

void PixelcharImplOpenGLRendererDestroy(PixelcharImplOpenGLRenderer* pRenderer);

bool PixelcharImplOpenGLFontCreate(
	PixelcharImplOpenGLFactory* pFactory,
	PixelcharFont* pSourceFont,
	PixelcharImplOpenGLFont* pFont,
	PixelcharLogCallback logCallback,
	void* logCallbackUserParam
);

void PixelcharImplOpenGLFontDestroy(PixelcharImplOpenGLFont* pFont);

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
);

#endif
