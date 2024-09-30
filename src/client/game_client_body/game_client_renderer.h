#pragma once

struct game_renderer {
	unsigned int fbo;
	unsigned int texture;

	unsigned int vbo;
	unsigned int vao;
	unsigned int ibo;

	int buffer_size_uniform_loaction;
	int color_uniform_location;

	unsigned int test_texture;

	struct {
		unsigned int default_shader_program;
	} shaders;

};

struct game_client;

void renderer_init(struct game_client* game);

void client_renderer_adjust_size(struct game_client* game);

void client_render_world(struct game_client* game);

void renderer_exit(struct game_client* game);