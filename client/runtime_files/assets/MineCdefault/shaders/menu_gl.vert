#version 330

uniform ivec2 screen_size;

layout(location = 0) in ivec2 in_position;
layout(location = 1) in vec4 in_image_uv_or_color;
layout(location = 2) in int in_texture_index;

layout(location = 0) out vec4 image_uv_or_color;
layout(location = 1) flat out int texture_index;

void main() {
    
    image_uv_or_color = in_image_uv_or_color;
    texture_index = in_texture_index;
	
    gl_Position = vec4(
		2.0 * vec2(in_position) / vec2(screen_size) - 1.0,
		0.0,
		1.0
	);
    
}