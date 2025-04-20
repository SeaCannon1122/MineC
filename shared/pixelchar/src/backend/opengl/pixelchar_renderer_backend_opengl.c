#include "pixelchar_internal.h"

uint32_t pixelchar_renderer_backend_opengl_init(
	struct pixelchar_renderer* pcr,
	uint8_t* vertex_shader_custom_source,
	size_t vertex_shader_custom_source_size,
	uint8_t* fragment_shader_custom_source,
	size_t fragment_shader_custom_source_size
)
{
	glGenVertexArrays(1, &pcr->backends.opengl.vao);
	glBindVertexArray(pcr->backends.opengl.vao);

	glGenBuffers(1, &pcr->backends.opengl.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pcr->backends.opengl.ebo);

	GLuint indices[] = { 0, 1, 2, 1, 3, 2 };
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glGenBuffers(1, &pcr->backends.opengl.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, pcr->backends.opengl.vbo);
	glBufferData(GL_ARRAY_BUFFER, pcr->char_buffer_length * sizeof(struct internal_pixelchar), NULL, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(struct internal_pixelchar), (void*)offsetof(struct internal_pixelchar, bitmap_index));
	glVertexAttribDivisor(0, 1);

	glEnableVertexAttribArray(1);
	glVertexAttribIPointer(1, 1, GL_UNSIGNED_SHORT, sizeof(struct internal_pixelchar), (void*)offsetof(struct internal_pixelchar, masks));
	glVertexAttribDivisor(1, 1);

	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, sizeof(struct internal_pixelchar), (void*)offsetof(struct internal_pixelchar, font));
	glVertexAttribDivisor(2, 1);

	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE, sizeof(struct internal_pixelchar), (void*)offsetof(struct internal_pixelchar, font_resolution));
	glVertexAttribDivisor(3, 1);

	glEnableVertexAttribArray(4);
	glVertexAttribIPointer(4, 1, GL_UNSIGNED_BYTE, sizeof(struct internal_pixelchar), (void*)offsetof(struct internal_pixelchar, scale));
	glVertexAttribDivisor(4, 1);

	glEnableVertexAttribArray(5);
	glVertexAttribIPointer(5, 1, GL_UNSIGNED_BYTE, sizeof(struct internal_pixelchar), (void*)offsetof(struct internal_pixelchar, bitmap_width));
	glVertexAttribDivisor(5, 1);

	glEnableVertexAttribArray(6);
	glVertexAttribIPointer(6, 2, GL_INT, sizeof(struct internal_pixelchar), (void*)offsetof(struct internal_pixelchar, position));
	glVertexAttribDivisor(6, 1);

	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(struct internal_pixelchar), (void*)offsetof(struct internal_pixelchar, color));
	glVertexAttribDivisor(7, 1);

	glEnableVertexAttribArray(8);
	glVertexAttribPointer(8, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(struct internal_pixelchar), (void*)offsetof(struct internal_pixelchar, background_color));
	glVertexAttribDivisor(8, 1);

	glBindVertexArray(0);

	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vertex_shader, 1, &vertex_shader_custom_source, &vertex_shader_custom_source_size);
	glShaderSource(fragment_shader, 1, &fragment_shader_custom_source, &fragment_shader_custom_source_size);

	glCompileShader(vertex_shader);
	glCompileShader(fragment_shader);

	pcr->backends.opengl.shader_program = glCreateProgram();
	glAttachShader(pcr->backends.opengl.shader_program, vertex_shader);
	glAttachShader(pcr->backends.opengl.shader_program, fragment_shader);
	glLinkProgram(pcr->backends.opengl.shader_program);

	GLint status;
	glGetProgramiv(pcr->backends.opengl.shader_program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		char log[10000];
		glGetProgramInfoLog(pcr->backends.opengl.shader_program, sizeof(log), 0, log);

		printf("Shader Linker Error: %s\n", log);
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	pcr->backends.opengl.uniform_location_screen_size = glGetUniformLocation(pcr->backends.opengl.shader_program, "screen_size");
	pcr->backends.opengl.uniform_location_shadow_color_devisor = glGetUniformLocation(pcr->backends.opengl.shader_program, "shadow_color_devisor");
	pcr->backends.opengl.uniform_location_draw_mode = glGetUniformLocation(pcr->backends.opengl.shader_program, "draw_mode");

	pcr->backends_initialized |= PIXELCHAR_BACKEND_OPENGL_BIT;

	for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONTS; i++) if (pcr->fonts[i] != NULL)
		_pixelchar_font_backend_opengl_reference_add(pcr->fonts[i], pcr, i);

	return PIXELCHAR_SUCCESS;
}

void pixelchar_renderer_backend_opengl_deinit(struct pixelchar_renderer* pcr)
{
	if (pcr->backends_initialized & PIXELCHAR_BACKEND_OPENGL_BIT == 0) _DEBUG_CALLBACK_ERROR_RETURN("pixelchar_renderer_backend_opengl_deinit: opengl backend not initialized");

	glDeleteProgram(pcr->backends.opengl.shader_program);

	glDeleteBuffers(1, &pcr->backends.opengl.vbo);
	glDeleteBuffers(1, &pcr->backends.opengl.ebo);
	glDeleteVertexArrays(1, &pcr->backends.opengl.vao);

	for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONTS; i++) if (pcr->fonts[i] != NULL)
		_pixelchar_font_backend_opengl_reference_subtract(pcr->fonts[i]);

	pcr->backends_initialized &= (~PIXELCHAR_BACKEND_OPENGL_BIT);
}

void pixelchar_renderer_backend_opengl_render(struct pixelchar_renderer* pcr, uint32_t width, uint32_t height, float shadow_devisor_r, float shadow_devisor_g, float shadow_devisor_b, float shadow_devisor_a)
{
	if (pcr->backends_initialized & PIXELCHAR_BACKEND_OPENGL_BIT == 0) _DEBUG_CALLBACK_ERROR_RETURN("pixelchar_renderer_backend_opengl_render: opengl backend not initialized");

	if (pcr->char_count == 0) return;

	_pixelchar_renderer_render_convert_to_internal_characters(pcr);

	glUseProgram(pcr->backends.opengl.shader_program);

	for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONTS; i++)
	{
		if (pcr->fonts[i]) glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i, pcr->fonts[i]->backends.opengl.buffer);
	}

	glBindVertexArray(pcr->backends.opengl.vao);

	glBindBuffer(GL_ARRAY_BUFFER, pcr->backends.opengl.vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, pcr->char_count * sizeof(struct internal_pixelchar), pcr->char_buffer);

	glUniform2i(pcr->backends.opengl.uniform_location_screen_size, width, height);
	glUniform4f(pcr->backends.opengl.uniform_location_shadow_color_devisor, shadow_devisor_r, shadow_devisor_g, shadow_devisor_b, shadow_devisor_a);

	for (uint32_t i = 0; i < 3; i++)
	{
		glUniform1ui(pcr->backends.opengl.uniform_location_draw_mode, i);

		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, pcr->char_count);
	}

	pcr->char_count = 0;
}
