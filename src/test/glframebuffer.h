#pragma once

#include <gl/glew.h>

struct frame_buffer {
	GLuint fbo;
	GLuint texture;
};

void frame_buffer_new(struct frame_buffer* buffer, int width, int height);

void frame_buffer_bind(struct frame_buffer* buffer);

void frame_buffer_set_size(struct frame_buffer* buffer, int width, int height);

void frame_buffer_destroy(struct frame_buffer* buffer);