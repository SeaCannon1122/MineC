#pragma once

#ifndef PIXELCHAR_RENDERER_BACKEND_OPENGL_H
#define PIXELCHAR_RENDERER_BACKEND_OPENGL_H

#include <stdint.h>
#include <glad/glad.h>

struct pixelchar_renderer_backend_opengl
{
	GLuint vbo;
	GLuint ebo;
	GLuint vao;

	GLuint shader_program;

	GLint uniform_location_screen_size;
	GLint uniform_location_shadow_color_devisor;
	GLint uniform_location_draw_mode;
};

uint32_t pixelchar_renderer_backend_opengl_init(
	struct pixelchar_renderer* pcr,
	uint8_t* vertex_shader_custom_source,
	size_t vertex_shader_custom_source_size,
	uint8_t* fragment_shader_custom_source,
	size_t fragment_shader_custom_source_size
);

void pixelchar_renderer_backend_opengl_deinit(struct pixelchar_renderer* pcr);

void pixelchar_renderer_backend_opengl_render(struct pixelchar_renderer* pcr, uint32_t width, uint32_t height, float shadow_devisor_r, float shadow_devisor_g, float shadow_devisor_b, float shadow_devisor_a);


#endif
