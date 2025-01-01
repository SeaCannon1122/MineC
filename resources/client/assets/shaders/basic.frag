#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 0, binding = 0) uniform texture2D images[];
layout(set = 0, binding = 1) uniform sampler samplers[];

layout(location = 0) in vec2 image_uv;
layout(location = 1) flat in int texture_index_or_color;
layout(location = 2) flat in int sampler_index;

layout(location = 0) out vec4 fragmentColor;

void main() {
    
    if (sampler_index != -1)
        fragmentColor = texture(sampler2D(images[texture_index_or_color], samplers[sampler_index]), image_uv);
    else {
        float r = float((texture_index_or_color >> 24) & 0xFFu) / 255.0;
        float g = float((texture_index_or_color >> 16) & 0xFFu) / 255.0;
        float b = float((texture_index_or_color >> 8) & 0xFFu) / 255.0;
        float a = float(texture_index_or_color & 0xFFu) / 255.0;
        fragmentColor = vec4(r, g, b, a);
    }
}