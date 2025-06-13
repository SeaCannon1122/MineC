#version 330
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 0, binding = 0) uniform texture2D images[];
layout(set = 0, binding = 1) uniform sampler samplers[];

layout(location = 0) in vec4 image_uv_or_color;
layout(location = 1) flat in int texture_index;

layout(location = 0) out vec4 fragmentColor;

void main() {
    
    if (texture_index == -1)
        fragmentColor = image_uv_or_color;
    else
        fragmentColor = texture(sampler2D(images[texture_index_or_color], samplers[sampler_index]), image_uv_or_color.xy);
    
}