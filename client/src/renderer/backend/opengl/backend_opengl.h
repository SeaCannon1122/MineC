#pragma once

#ifndef MINEC_CLIENT_RENDERER_BACKEND_VULKAN_BACKEND_OPENGL_H
#define MINEC_CLIENT_RENDERER_BACKEND_VULKAN_BACKEND_OPENGL_H

#include <GL/glcorearb.h>
#include <pixelchar/backend/backend_opengl.h>
#include <minec_client.h>

void* renderer_backend_get_window_context();

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
	float last_frame_time;

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
		GLuint* textures;
	} textures;

	struct
	{
		bool usable;
		uint32_t backend_index;
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




static void _minec_client_retrieve_log_opengl_errors(struct minec_client* client, struct renderer_backend_opengl_base* base, uint32_t* p_result, uint8_t* function, uint8_t* file, uint32_t line, uint8_t* action)
{
	if (*p_result != MINEC_CLIENT_SUCCESS) return;

	GLenum error;
	for (uint32_t error_count = 0; (error = base->func.glGetError()) != GL_NO_ERROR; error_count++)
	{
		*p_result = MINEC_CLIENT_ERROR;

		if (error_count > 16)
		{
			_minec_client_log_debug_error(client, function, file, line, "'%s' failed with more then 16 glErrors", action);
			return;
		}

		uint8_t* error_name = "Unknown";
		switch (error)
		{
		case GL_INVALID_ENUM: { error_name = "GL_INVALID_ENUM"; break; }
		case GL_INVALID_VALUE: { error_name = "GL_INVALID_VALUE"; break; }
		case GL_INVALID_OPERATION: { error_name = "GL_INVALID_OPERATION"; break; }
		case GL_OUT_OF_MEMORY: { error_name = "GL_OUT_OF_MEMORY"; break; }
		case GL_INVALID_FRAMEBUFFER_OPERATION: { error_name = "GL_INVALID_FRAMEBUFFER_OPERATION"; break; }
		case GL_CONTEXT_LOST: { 
			minec_client_log_error(client, "[FATAL] OpenGL Context lost. Crashing ...");
			minec_client_nuke_destroy_kill_crush_annihilate_process_exit(client);
		}
		}

		_minec_client_log_debug_error(client, function, file, line, "'%s' failed with glError 0x%x '%s'", action, error, error_name);
	}
}

#define minec_client_retrieve_log_opengl_errors(client, base, p_result, action) _minec_client_retrieve_log_opengl_errors(client, base, p_result, __func__, __FILE__, __LINE__, action)


#endif
