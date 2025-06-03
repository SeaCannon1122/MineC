#pragma once

#ifndef MINEC_CLIENT_RENDERER_BACKEND_VULKAN_BACKEND_OPENGL_H
#define MINEC_CLIENT_RENDERER_BACKEND_VULKAN_BACKEND_OPENGL_H

#include "../backend_internal.h"

#include <GL/glcorearb.h>
#include <pixelchar/backend/backend_opengl.h>

static void renderer_backend_opengl_log(struct minec_client* client, uint8_t* message, ...)
{
	va_list args;
	va_start(args, message);
	minec_client_log_v(client, "[RENDERER][OPENGL]", message, args);
	va_end(args);
}

#define OPENGL_FRAME_COUNT 3
#define OPENGL_MENU_GUI_QUAD_COUNT 128

struct renderer_backend_opengl_base
{
	struct
	{
#ifdef  MINEC_CLIENT_RENDERER_BACKEND_DEBUG
		PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;
		PFNGLDEBUGMESSAGECONTROLPROC glDebugMessageControl;
#endif
		PFNGLENABLEPROC glEnable;
		PFNGLDISABLEPROC glDisable;
		PFNGLGETERRORPROC glGetError;
		PFNGLGETSTRINGPROC glGetString;
		PFNGLGETSTRINGIPROC glGetStringi;
		PFNGLGETINTEGERVPROC glGetIntegerv;

		//framebuffer
		PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
		PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
		PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
		PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
		PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;

		//textures
		PFNGLGENTEXTURESPROC glGenTextures;
		PFNGLDELETETEXTURESPROC glDeleteTextures;
		PFNGLBINDTEXTUREPROC glBindTexture;
		PFNGLTEXIMAGE2DPROC glTexImage2D;
		PFNGLTEXPARAMETERIPROC glTexParameteri;

		//render buffer
		PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
		PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
		PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
		PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
		PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;

		//shader
		PFNGLCREATEPROGRAMPROC glCreateProgram;
		PFNGLDELETEPROGRAMPROC glDeleteProgram;
		PFNGLCREATESHADERPROC glCreateShader;
		PFNGLDELETESHADERPROC glDeleteShader;
		PFNGLSHADERSOURCEPROC glShaderSource;
		PFNGLCOMPILESHADERPROC glCompileShader;
		PFNGLATTACHSHADERPROC glAttachShader;
		PFNGLLINKPROGRAMPROC glLinkProgram;
		PFNGLUSEPROGRAMPROC glUseProgram;

		//VAO
		PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
		PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
		PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
		PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
		PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
		PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;


		//Buffer
		PFNGLGENBUFFERSPROC glGenBuffers;
		PFNGLDELETEBUFFERSPROC glDeleteBuffers;
		PFNGLBINDBUFFERPROC glBindBuffer;
		PFNGLBUFFERDATAPROC glBufferData;
		PFNGLBUFFERSUBDATAPROC glBufferSubData;
		PFNGLMAPBUFFERPROC glMapBuffer;
		PFNGLMAPBUFFERRANGEPROC glMapBufferRange;
		PFNGLUNMAPBUFFERPROC glUnmapBuffer;


		//blend
		PFNGLBLENDFUNCPROC glBlendFunc;
		PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate;
		PFNGLBLENDEQUATIONPROC glBlendEquation;
		PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate;
		PFNGLBLENDCOLORPROC glBlendColor;

		//stencil
		PFNGLSTENCILFUNCPROC glStencilFunc;
		PFNGLSTENCILOPPROC glStencilOp;
		PFNGLSTENCILMASKPROC glStencilMask;
		PFNGLCLEARSTENCILPROC glClearStencil;

		PFNGLVIEWPORTPROC glViewport;
		PFNGLCLEARPROC glClear;
		PFNGLCLEARCOLORPROC glClearColor;
		PFNGLDEPTHFUNCPROC glDepthFunc;
		PFNGLDEPTHMASKPROC glDepthMask;
		PFNGLCULLFACEPROC glCullFace;
		PFNGLFRONTFACEPROC glFrontFace;

		//draw
		PFNGLDRAWARRAYSPROC glDrawArrays;
		PFNGLDRAWELEMENTSPROC glDrawElements;
		PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced;
		PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced;

	} func;

	uint32_t fps;
	atomic_(uint32_t) fps_new;

	struct
	{
		struct
		{
			GLuint vao;
			GLuint vbo[OPENGL_FRAME_COUNT];
		} pass;
	} menu_gui;

};

struct renderer_backend_opengl_pipelines_resources
{
	struct {
		uint32_t (*handles)[2];
		GLuint textures;
	} textures;

	struct
	{
		bool usable;
	} pixelchar_renderer;

	struct
	{
		struct
		{
			GLuint program;
			bool usable;
		} pass;
	} menu_gui;

};

static uint32_t gl_error_check_log(struct minec_client* client, struct renderer_backend_opengl_base* base, uint8_t* message)
{
	uint32_t result = MINEC_CLIENT_SUCCESS;

	GLenum error;
	for (uint32_t error_count = 0; (error = base->func.glGetError()) != GL_NO_ERROR; error_count++)
	{
		if (error_count > 10)
		{
			renderer_backend_opengl_log(client, "Detected more than 10 errors. Something weird is going on");
			return MINEC_CLIENT_ERROR;
		}

		uint8_t* error_name = "Unknown";
		switch (error)
		{
		case GL_INVALID_ENUM: { error_name = "GL_INVALID_ENUM"; break; }
		case GL_INVALID_VALUE: { error_name = "GL_INVALID_VALUE"; break; }
		case GL_INVALID_OPERATION: { error_name = "GL_INVALID_OPERATION"; break; }
		case GL_STACK_OVERFLOW: { error_name = "GL_STACK_OVERFLOW"; break; }
		case GL_STACK_UNDERFLOW: { error_name = "GL_STACK_UNDERFLOW"; break; }
		case GL_OUT_OF_MEMORY: { error_name = "GL_OUT_OF_MEMORY"; break; }
		case GL_INVALID_FRAMEBUFFER_OPERATION: { error_name = "GL_INVALID_FRAMEBUFFER_OPERATION"; break; }
		case GL_CONTEXT_LOST: { error_name = "GL_CONTEXT_LOST "; break; }
		}

		renderer_backend_opengl_log(client, "Error 0x%x '%s': %s", error, error_name, message);
		result = MINEC_CLIENT_ERROR;
	}
	return result;
}

#endif
