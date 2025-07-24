#pragma once

#ifndef MINEC_CLIENT_RENDERER_BACKEND_VULKAN_BACKEND_OPENGL_H
#define MINEC_CLIENT_RENDERER_BACKEND_VULKAN_BACKEND_OPENGL_H

#include <GL/glcorearb.h>

#define OPENGL RENDERER.BACKEND.state.opengl

struct renderer_backend_opengl
{
	struct
	{
#ifdef  MINEC_CLIENT_DEBUG_LOG
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

	uint8_t* extensions[1024];
	GLint extension_count;
};

struct minec_client;

uint32_t renderer_backend_opengl_create(struct minec_client* client);
void renderer_backend_opengl_destroy(struct minec_client* client);
bool _opengl_error_get_log(struct minec_client* client, uint8_t* action, uint8_t* function, uint8_t* file, uint32_t line);
#define opengl_error_get_log(client, action) _opengl_error_get_log(client, action, __func__, __FILE__, __LINE__)
uint32_t opengl_errors_clear(struct minec_client* client);


#endif