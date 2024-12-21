#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 0, binding = 0) uniform sampler2D textures[];

layout(push_constant) uniform PushConstants {
    int screen_width;
    int screen_height;
};

layout(location = 0) in vec2 image_uv;
layout(location = 1) flat in uint texture_index;

layout(location = 0) out vec4 fragmentColor;

void main() {
    fragmentColor = texture(textures[texture_index], image_uv);
}
