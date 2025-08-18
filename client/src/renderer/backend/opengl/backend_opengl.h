#pragma once

#ifndef MINEC_CLIENT_RENDERER_BACKEND_VULKAN_BACKEND_OPENGL_H
#define MINEC_CLIENT_RENDERER_BACKEND_VULKAN_BACKEND_OPENGL_H

#include <GL/glcorearb.h>
#include <pixelchar/backend/backend_opengl.h>

#define OPENGL RENDERER.backend.state.opengl
#define OPENGL_RESOURCE_FRAME_COUNT 2

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


struct renderer_backend_opengl
{
	struct
	{
#define OPENGL_FUNCTION(signature, name) signature name;
		OPENGL_FUNCTION_LIST
#undef OPENGL_FUNCTION
	} func;

	GLubyte* extensions[1024];
	GLint extension_count;

	float last_frame_time;
	uint32_t resource_frame_index;
};

struct minec_client;

uint32_t renderer_backend_opengl_create(struct minec_client* client);
void renderer_backend_opengl_destroy(struct minec_client* client);

uint32_t renderer_backend_opengl_reload_assets(struct minec_client* client);

uint32_t renderer_backend_opengl_switch_device(struct minec_client* client, uint32_t device_index);
uint32_t renderer_backend_opengl_set_vsync(struct minec_client* client, bool vsync);
uint32_t renderer_backend_opengl_set_fps(struct minec_client* client, uint32_t fps);
uint32_t renderer_backend_opengl_set_max_mipmap_level_count(struct minec_client* client, uint32_t max_mipmap_level_count);

uint32_t renderer_backend_opengl_frame_begin(struct minec_client* client);
uint32_t renderer_backend_opengl_frame_menu(struct minec_client* client);
uint32_t renderer_backend_opengl_frame_end(struct minec_client* client);

bool _opengl_error_get_log(struct minec_client* client, uint8_t* action, uint8_t* function, uint8_t* file, uint32_t line);
#define opengl_error_get_log(client, action) _opengl_error_get_log(client, action, __func__, __FILE__, __LINE__)
uint32_t opengl_errors_clear(struct minec_client* client);


#endif