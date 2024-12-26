#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 0, binding = 0) uniform texture2D images[];
layout(set = 0, binding = 1) uniform sampler samplers[];

layout(push_constant) uniform PushConstants {
    int screen_width;
    int screen_height;
};

layout(location = 0) in vec2 image_uv;
layout(location = 1) flat in uint texture_index;
layout(location = 2) flat in uint sampler_index;

layout(location = 0) out vec4 fragmentColor;

void main() {
    fragmentColor = texture(sampler2D(images[texture_index], samplers[sampler_index]), image_uv);
    
}
