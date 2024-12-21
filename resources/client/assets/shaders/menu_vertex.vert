#version 450

layout(push_constant) uniform PushConstants {
	int screen_width;
	int screen_height;
};

layout(location = 0) in ivec2 in_position;
layout(location = 1) in vec2 in_image_uv;
layout(location = 2) in uint in_testure_index;

layout(location = 0) out vec2 image_uv;
layout(location = 1) flat out uint testure_index;

void main() {
    
    image_uv = in_image_uv;
	
    gl_Position = vec4(
		-1.0 + 2.0 * float(in_position.x) / float(screen_width),
		-1.0 + 2.0 * float(in_position.y) / float(screen_height),
		1.0,
		1.0
	);
    
}