#include "backend_opengl.h"

uint32_t create_framebuffers(struct minec_client* client, struct renderer_backend_opengl_base* base)
{
	for (uint32_t i = 0; i < OPENGL_FRAME_COUNT; i++)
	{
		base->func.glGenFramebuffers(1, &base->framebuffers[i].game.fbo);
		base->func.glGenFramebuffers(1, &base->framebuffers[i].ldr_convert.fbo);
		base->func.glGenFramebuffers(1, &base->framebuffers[i].menu_gui.fbo);
	}
	

	return MINEC_CLIENT_SUCCESS;
}

void destroy_framebuffers(struct minec_client* client, struct renderer_backend_opengl_base* base)
{
	for (uint32_t i = 0; i < OPENGL_FRAME_COUNT; i++)
	{
		base->func.glDeleteFramebuffers(1, &base->framebuffers[i].game.fbo);
		base->func.glDeleteFramebuffers(1, &base->framebuffers[i].ldr_convert.fbo);
		base->func.glDeleteFramebuffers(1, &base->framebuffers[i].menu_gui.fbo);
	}
}

uint32_t create_framebuffer_attachments(struct minec_client* client, struct renderer_backend_opengl_base* base, uint32_t winodw_width, uint32_t cwindow_height)
{
	for (uint32_t i = 0; i < OPENGL_FRAME_COUNT; i++)
	{
		//game
		base->func.glGenTextures(1, &base->framebuffers[i].game.color_buffer);
		base->func.glBindTexture(GL_TEXTURE_2D, base->framebuffers[i].game.color_buffer);
		base->func.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, winodw_width, cwindow_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		base->func.glBindFramebuffer(GL_FRAMEBUFFER, base->framebuffers[i].game.fbo);
		base->func.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, base->framebuffers[i].game.color_buffer, 0);

		//menu
		base->func.glGenTextures(1, &base->framebuffers[i].menu_gui.color_buffer);
		base->func.glBindTexture(GL_TEXTURE_2D, base->framebuffers[i].menu_gui.color_buffer);
		base->func.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, winodw_width, cwindow_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		base->func.glBindFramebuffer(GL_FRAMEBUFFER, base->framebuffers[i].menu_gui.fbo);
		base->func.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, base->framebuffers[i].menu_gui.color_buffer, 0);

		base->func.glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	return MINEC_CLIENT_SUCCESS;
}

void destroy_framebuffer_attachments(struct minec_client* client, struct renderer_backend_opengl_base* base, uint32_t winodw_width, uint32_t cwindow_height)
{
	for (uint32_t i = 0; i < OPENGL_FRAME_COUNT; i++)
	{
		base->func.glDeleteTextures(1, &base->framebuffers[i].game.color_buffer);
		base->func.glDeleteTextures(1, &base->framebuffers[i].menu_gui.color_buffer);
	}
}


