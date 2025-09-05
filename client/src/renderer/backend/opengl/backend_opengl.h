#pragma once

#ifndef MINEC_CLIENT_RENDERER_BACKEND_OPENGL_BACKEND_OPENGL_H
#define MINEC_CLIENT_RENDERER_BACKEND_OPENGL_BACKEND_OPENGL_H

#include <cwindow/cwindow.h>

#define OPENGL_FUNC base->opengl.func
#define OPENGL_RESOURCE_FRAME_COUNT 3

#define OPENGL_FUNCTION_LIST_NO_DEBUG \
	OPENGL_FUNCTION(PFNGLENABLEPROC, glEnable)\
	OPENGL_FUNCTION(PFNGLDISABLEPROC, glDisable)\
	OPENGL_FUNCTION(PFNGLGETERRORPROC, glGetError)\
	OPENGL_FUNCTION(PFNGLGETSTRINGPROC, glGetString)\
	OPENGL_FUNCTION(PFNGLGETSTRINGIPROC, glGetStringi)\
	OPENGL_FUNCTION(PFNGLGETINTEGERVPROC, glGetIntegerv)\
	\
	OPENGL_FUNCTION(PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers)\
	OPENGL_FUNCTION(PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers)\
	OPENGL_FUNCTION(PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer)\
	OPENGL_FUNCTION(PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D)\
	OPENGL_FUNCTION(PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus)\
	\
	OPENGL_FUNCTION(PFNGLGENTEXTURESPROC, glGenTextures)\
	OPENGL_FUNCTION(PFNGLDELETETEXTURESPROC, glDeleteTextures)\
	OPENGL_FUNCTION(PFNGLBINDTEXTUREPROC, glBindTexture)\
	OPENGL_FUNCTION(PFNGLTEXIMAGE2DPROC, glTexImage2D)\
	OPENGL_FUNCTION(PFNGLTEXPARAMETERIPROC, glTexParameteri)\
	\
	OPENGL_FUNCTION(PFNGLGENRENDERBUFFERSPROC, glGenRenderbuffers)\
	OPENGL_FUNCTION(PFNGLDELETERENDERBUFFERSPROC, glDeleteRenderbuffers)\
	OPENGL_FUNCTION(PFNGLBINDRENDERBUFFERPROC, glBindRenderbuffer)\
	OPENGL_FUNCTION(PFNGLRENDERBUFFERSTORAGEPROC, glRenderbufferStorage)\
	OPENGL_FUNCTION(PFNGLFRAMEBUFFERRENDERBUFFERPROC, glFramebufferRenderbuffer)\
	\
	OPENGL_FUNCTION(PFNGLCREATEPROGRAMPROC, glCreateProgram)\
	OPENGL_FUNCTION(PFNGLDELETEPROGRAMPROC, glDeleteProgram)\
	OPENGL_FUNCTION(PFNGLCREATESHADERPROC, glCreateShader)\
	OPENGL_FUNCTION(PFNGLDELETESHADERPROC, glDeleteShader)\
	OPENGL_FUNCTION(PFNGLSHADERSOURCEPROC, glShaderSource)\
	OPENGL_FUNCTION(PFNGLCOMPILESHADERPROC, glCompileShader)\
	OPENGL_FUNCTION(PFNGLATTACHSHADERPROC, glAttachShader)\
	OPENGL_FUNCTION(PFNGLLINKPROGRAMPROC, glLinkProgram)\
	OPENGL_FUNCTION(PFNGLUSEPROGRAMPROC, glUseProgram)\
	\
	OPENGL_FUNCTION(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays)\
	OPENGL_FUNCTION(PFNGLDELETEVERTEXARRAYSPROC, glDeleteVertexArrays)\
	OPENGL_FUNCTION(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray)\
	OPENGL_FUNCTION(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer)\
	OPENGL_FUNCTION(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray)\
	OPENGL_FUNCTION(PFNGLDISABLEVERTEXATTRIBARRAYPROC, glDisableVertexAttribArray)\
	\
	OPENGL_FUNCTION(PFNGLGENBUFFERSPROC, glGenBuffers)\
	OPENGL_FUNCTION(PFNGLDELETEBUFFERSPROC, glDeleteBuffers)\
	OPENGL_FUNCTION(PFNGLBINDBUFFERPROC, glBindBuffer)\
	OPENGL_FUNCTION(PFNGLBUFFERDATAPROC, glBufferData)\
	OPENGL_FUNCTION(PFNGLBUFFERSUBDATAPROC, glBufferSubData)\
	OPENGL_FUNCTION(PFNGLMAPBUFFERPROC, glMapBuffer)\
	OPENGL_FUNCTION(PFNGLMAPBUFFERRANGEPROC, glMapBufferRange)\
	OPENGL_FUNCTION(PFNGLUNMAPBUFFERPROC, glUnmapBuffer)\
	\
	OPENGL_FUNCTION(PFNGLBLENDFUNCPROC, glBlendFunc)\
	OPENGL_FUNCTION(PFNGLBLENDFUNCSEPARATEPROC, glBlendFuncSeparate)\
	OPENGL_FUNCTION(PFNGLBLENDEQUATIONPROC, glBlendEquation)\
	OPENGL_FUNCTION(PFNGLBLENDEQUATIONSEPARATEPROC, glBlendEquationSeparate)\
	OPENGL_FUNCTION(PFNGLBLENDCOLORPROC, glBlendColor)\
	\
	OPENGL_FUNCTION(PFNGLSTENCILFUNCPROC, glStencilFunc)\
	OPENGL_FUNCTION(PFNGLSTENCILOPPROC, glStencilOp)\
	OPENGL_FUNCTION(PFNGLSTENCILMASKPROC, glStencilMask)\
	OPENGL_FUNCTION(PFNGLCLEARSTENCILPROC, glClearStencil)\
	\
	OPENGL_FUNCTION(PFNGLVIEWPORTPROC, glViewport)\
	OPENGL_FUNCTION(PFNGLCLEARPROC, glClear)\
	OPENGL_FUNCTION(PFNGLCLEARCOLORPROC, glClearColor)\
	OPENGL_FUNCTION(PFNGLDEPTHFUNCPROC, glDepthFunc)\
	OPENGL_FUNCTION(PFNGLDEPTHMASKPROC, glDepthMask)\
	OPENGL_FUNCTION(PFNGLCULLFACEPROC, glCullFace)\
	OPENGL_FUNCTION(PFNGLFRONTFACEPROC, glFrontFace)\
	\
	OPENGL_FUNCTION(PFNGLDRAWARRAYSPROC, glDrawArrays)\
	OPENGL_FUNCTION(PFNGLDRAWELEMENTSPROC, glDrawElements)\
	OPENGL_FUNCTION(PFNGLDRAWARRAYSINSTANCEDPROC, glDrawArraysInstanced)\
	OPENGL_FUNCTION(PFNGLDRAWELEMENTSINSTANCEDPROC, glDrawElementsInstanced)\

#define OPENGL_FUNCTION_LIST_DEBUG \
	OPENGL_FUNCTION(PFNGLDEBUGMESSAGECALLBACKPROC, glDebugMessageCallback)\
	OPENGL_FUNCTION(PFNGLDEBUGMESSAGECONTROLPROC, glDebugMessageControl)\

#ifdef MINEC_CLIENT_DEBUG_LOG

#define OPENGL_FUNCTION_LIST \
	OPENGL_FUNCTION_LIST_DEBUG \
	OPENGL_FUNCTION_LIST_NO_DEBUG \

#else

#define OPENGL_FUNCTION_LIST \
	OPENGL_FUNCTION_LIST_NO_DEBUG \

#endif

struct renderer_backend_opengl_base	{ 

	struct
	{
#define OPENGL_FUNCTION(signature, name) signature name;
		OPENGL_FUNCTION_LIST
#undef OPENGL_FUNCTION
	} func;

	cwindow_context* window_context;
	cwindow* window;

	uint32_t resource_frame_index;
};

struct renderer_backend_opengl_device { 
	int _empty;
};

struct renderer_backend_opengl_swapchain {
	int _empty;
};

struct minec_client;
struct renderer_backend_device_infos;
struct renderer_backend_base;
struct renderer_backend_device;
struct renderer_backend_swapchain;

struct renderer_backend_info* renderer_backend_opengl_get_info(
	struct minec_client* client
);

uint32_t renderer_backend_opengl_base_create(
	struct minec_client* client,
	cwindow_context* window_context,
	cwindow* window,
	struct renderer_backend_device_infos* device_infos,
	struct renderer_backend_base* base
);
void renderer_backend_opengl_base_destroy(
	struct minec_client* client,
	struct renderer_backend_base* base
);

uint32_t renderer_backend_opengl_device_create(
	struct minec_client* client,
	uint32_t device_index,
	struct renderer_backend_device* device
);

void renderer_backend_opengl_device_destroy(
	struct minec_client* client,
	struct renderer_backend_device* device
);

uint32_t renderer_backend_opengl_swapchain_create(
	struct minec_client* client,
	struct renderer_backend_device* device,
	uint32_t width,
	uint32_t height,
	bool vsync,
	bool triple_buffering,
	struct renderer_backend_swapchain* swapchain
);

void renderer_backend_opengl_swapchain_destroy(
	struct minec_client* client,
	struct renderer_backend_device* device,
	struct renderer_backend_swapchain* swapchain
);

uint32_t renderer_backend_opengl_frame_start(
	struct minec_client* client,
	struct renderer_backend_device* device
);

uint32_t renderer_backend_opengl_frame_submit(
	struct minec_client* client,
	struct renderer_backend_device* device
);

bool _opengl_error_get_log(struct minec_client* client, struct renderer_backend_base* base, uint8_t* action, uint8_t* function, uint8_t* file, uint32_t line);
#define opengl_error_get_log(client, action) _opengl_error_get_log(client, base, action, __func__, __FILE__, __LINE__)
uint32_t _opengl_errors_clear(struct minec_client* client, struct renderer_backend_base* base);
#define opengl_errors_clear(client) _opengl_errors_clear(client, base);


#endif