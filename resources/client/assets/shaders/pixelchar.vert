#version 450

uint VULKAN_PIXELFONT_INVALID = 255;

uint PIXELCHAR_MASK_UNDERLINE = 1;
uint PIXELCHAR_MASK_CURSIVE = 2;
uint PIXELCHAR_MASK_SHADOW = 4;
uint PIXELCHAR_MASK_BACKGROUND = 8;

layout(push_constant) uniform PushConstants 
{
    ivec2 screen_size;
    vec4 color_devisor; 
    uint draw_mode;
};

layout(location = 0) in uint in_bitmap_index;
layout(location = 1) in uint in_masks;
layout(location = 2) in uint in_font;
layout(location = 3) in uint in_font_resolution;
layout(location = 4) in uint in_scale;
layout(location = 5) in uint in_bitmap_width;
layout(location = 6) in ivec2 in_position;
layout(location = 7) in vec4 in_color;
layout(location = 8) in vec4 in_background_color;

layout(location = 0) flat out uvec4 bitmap_index_I_masks_I_font_I_font_resolution;
layout(location = 1) flat out uint scale;
layout(location = 2) out vec2 fragment_position;
layout(location = 3) out vec4 color;
layout(location = 4) out vec4 background_color;

void main()
{

    color = in_color;
    background_color = in_background_color;
    bitmap_index_I_masks_I_font_I_font_resolution = uvec4(in_bitmap_index, in_masks, in_font, in_font_resolution);
    scale = in_scale;

    if ((draw_mode == 0 && (in_masks & PIXELCHAR_MASK_BACKGROUND) == 0) || (draw_mode == 1 && (in_masks & PIXELCHAR_MASK_SHADOW) == 0) || in_font == VULKAN_PIXELFONT_INVALID)
        gl_Position = vec4(0.0, 0.0, 0.0, 0.0);
    else
    {
        ivec2 dimensions = ivec2((in_bitmap_width * in_scale * 8 + in_font_resolution - 1) / in_font_resolution, in_scale * 8);
        ivec2 vertex_position;
    
        if (gl_VertexIndex % 4 == 0)
            vertex_position = ivec2(-dimensions.y / 16,                       -(dimensions.y + 15) / 16);
        else if (gl_VertexIndex % 4 == 1)
            vertex_position = ivec2(-dimensions.y / 16,                        19 * dimensions.y / 16);
        else if (gl_VertexIndex % 4 == 2)
            vertex_position = ivec2( dimensions.x + (dimensions.y + 15) / 16, -(dimensions.y + 15) / 16);
        else if (gl_VertexIndex % 4 == 3)
            vertex_position = ivec2( dimensions.x + (dimensions.y + 15) / 16,  19 * dimensions.y / 16);

        fragment_position = vec2(vertex_position);
        
        vertex_position += in_position.xy;
        
        //if ((masks & PIXELCHAR_MASK_CURSIVE) != 0)
        //    vertex_position.x += (dimensions.y * 7 / 8 + float(in_position.y) - vertex_position.y) / 2.0;
        
        if (draw_mode == 1)
            vertex_position += 3 * int(in_scale) / 5 + 1;
        
        gl_Position = vec4(vec2(vertex_position * 2) / vec2(screen_size) - 1.0, 0.0, 1.0);
    }
}