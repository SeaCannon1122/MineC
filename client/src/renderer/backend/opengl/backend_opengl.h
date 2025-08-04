#pragma once

#ifndef MINEC_CLIENT_RENDERER_BACKEND_VULKAN_BACKEND_OPENGL_H
#define MINEC_CLIENT_RENDERER_BACKEND_VULKAN_BACKEND_OPENGL_H

#include <GL/glcorearb.h>
#include <pixelchar/backend/backend_opengl.h>

#define OPENGL RENDERER.backend.state.opengl
#define OPENGL_RESOURCE_FRAME_COUNT 2

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

	GLubyte* extensions[1024];
	GLint extension_count;

	float last_frame_time;
	uint32_t resource_frame_index;
};

struct minec_client;

uint32_t renderer_backend_opengl_create(struct minec_client* client);
void renderer_backend_opengl_destroy(struct minec_client* client);

uint32_t renderer_backend_opengl_frame(struct minec_client* client);

uint32_t renderer_backend_opengl_switch_device(struct minec_client* client, uint32_t device_index);
uint32_t renderer_backend_opengl_reload_resources(struct minec_client* client);
uint32_t renderer_backend_opengl_set_vsync(struct minec_client* client, bool vsync);
uint32_t renderer_backend_opengl_set_fps(struct minec_client* client, uint32_t fps);
uint32_t renderer_backend_opengl_set_max_mipmap_level_count(struct minec_client* client, uint32_t max_mipmap_level_count);

bool _opengl_error_get_log(struct minec_client* client, uint8_t* action, uint8_t* function, uint8_t* file, uint32_t line);
#define opengl_error_get_log(client, action) _opengl_error_get_log(client, action, __func__, __FILE__, __LINE__)
uint32_t opengl_errors_clear(struct minec_client* client);


#endif