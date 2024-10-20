#version 430 core

#extension GL_ARB_bindless_texture : require

layout(std430, binding = 0) buffer handles {
    uvec2 resource_handles[];
};

uniform int screen_width;
uniform int screen_height;

layout(location = 0) out vec4 frag_color;

flat in uvec4 pixel_char;
in vec2 pixel_position;

void main() {

    uvec2 font_handle = resource_handles[0];

    float red = float((pixel_char.x >> 16) & 0xFF) / 255.0;
    float green = float((pixel_char.x >> 8) & 0xFF) / 255.0;
    float blue = float((pixel_char.x >> 0) & 0xFF) / 255.0;

    frag_color = vec4(blue, green, red, 1.0);

}
