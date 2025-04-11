#version 450

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

layout(location = 0) in vec4 in_color;
layout(location = 1) in vec4 in_background_color;
layout(location = 2) in uint in_bitmap_index;
layout(location = 3) in uint in_font;
layout(location = 4) in uint in_bitmap_width;
layout(location = 5) in uint in_masks;
layout(location = 6) in uint in_font_resolution;
layout(location = 7) in ivec4 in_position;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 background_color;
layout(location = 2) flat out uvec4 font_resolution_index_width;
layout(location = 3) flat out uint masks;
layout(location = 4) out vec2 fragment_position;
layout(location = 5) flat out ivec2 char_dims;

void main()
{

    color = in_color;
    background_color = in_background_color;
    font_bitmap_resolution_index_width = uvec4(in_font, in_font_resolution, in_bitmap_index, in_bitmap_width);
    masks = in_masks;
	
    if ((draw_mode == 0 && (masks & PIXELCHAR_MASK_BACKGROUND) == 0) || (draw_mode == 1 && (masks & PIXELCHAR_MASK_SHADOW) == 0))
        gl_Position = vec4(0.0, 0.0, 0.0, 0.0);
    else
    {
        char_dims = in_position.zw - in_position.xy;
    
        vec2 vertex_position;
    
        if (gl_VertexIndex % 4 == 0)
            vertex_position = vec2(-char_dims.y / 16, -(char_dims.y + 15) / 16);
        else if (gl_VertexIndex % 4 == 1)
            vertex_position = vec2(-char_dims.y / 16, char_dims.y + char_dims.y / 16);
        else if (gl_VertexIndex % 4 == 2)
            vertex_position = vec2(char_dims.x + (char_dims.y + 15) / 16, -(char_dims.y + 15) / 16);
        else if (gl_VertexIndex % 4 == 3)
            vertex_position = vec2(char_dims.x + (char_dims.y + 15) / 16, char_dims.y + char_dims.y / 16);

        fragment_position = vertex_position / float(char_dims.y);
        
        vertex_position += vec2(in_position.xy);
        
        if ((masks & PIXELCHAR_MASK_CURSIVE) != 0)
            vertex_position.x += (float(char_dims.y * 7 / 8 + in_position.y) - vertex_position.y) / 2.0;
        
        if (draw_mode == 1)
            vertex_position += (char_dims.y + 7) / 8;
        
        gl_Position = vec4((vertex_position * 2.0) / vec2(screen_size) - 1.0, 1.0, 1.0);
    }
}