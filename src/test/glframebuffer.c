#include "glframebuffer.h"
#include "general/platformlib/opengl_rendering.h"

void frame_buffer_new(struct frame_buffer* buffer, int width, int height) {

	GLCall(glGenFramebuffers(1, &buffer->fbo));
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, buffer->fbo));

	GLCall(glGenTextures(1, &buffer->texture));
	GLCall(glBindTexture(GL_TEXTURE_2D, buffer->texture));
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

	GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer->texture, 0));
}

void frame_buffer_bind(struct frame_buffer* buffer) {
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, buffer->fbo));
}

void frame_buffer_set_size(struct frame_buffer* buffer, int width, int height) {
	GLCall(glBindTexture(GL_TEXTURE_2D, buffer->texture));
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0));
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}

void frame_buffer_destroy(struct frame_buffer* buffer) {
	GLCall(glDeleteTextures(1, &buffer->texture));
	GLCall(glDeleteFramebuffers(1, &buffer->fbo));
}