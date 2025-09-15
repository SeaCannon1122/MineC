#version 410 core

#extension GL_ARB_shader_storage_buffer_object : enable

const int MODIFIER_BOLD_BIT = 1;
const int MODIFIER_ITALIC_BIT = 2;
const int MODIFIER_UNDERLINE_BIT = 4;
const int MODIFIER_STRIKETHROUGH_BIT = 8;
const int MODIFIER_SHADOW_BIT = 16;
const int MODIFIER_BACKGROUND_BIT = 32;

uniform mat4 screen_to_ndc;
uniform int scale;
uniform int font_resolution;
uniform uint draw_mode;

layout(location = 0) in ivec4 in_position_I_extent;
layout(location = 1) in ivec4 in_bitmap_index_I_bitmap_width_I_bitmap_thickness_I_modifiers;
layout(location = 2) in vec4 in_color;
layout(location = 3) in vec4 in_background_color;

flat out ivec4 bitmap_index_I_bitmap_width_I_bitmap_thickness_I_modifiers;;
out vec2 fragment_position;
out vec4 color;
out vec4 background_color;

void main()
{
    bitmap_index_I_bitmap_width_I_bitmap_thickness_I_modifiers = in_bitmap_index_I_bitmap_width_I_bitmap_thickness_I_modifiers;
    color = in_color;
    background_color = in_background_color;
    
    int bitmap_index = in_bitmap_index_I_bitmap_width_I_bitmap_thickness_I_modifiers.x;
    int modifiers = in_bitmap_index_I_bitmap_width_I_bitmap_thickness_I_modifiers.w;
    
    if ((draw_mode == 0 && (modifiers & MODIFIER_BACKGROUND_BIT) == 0) || (draw_mode == 1 && (modifiers & MODIFIER_SHADOW_BIT) == 0))
        gl_Position = vec4(2.0, 2.0, 0.0, 1.0);
    else
    {
        ivec2 vertex_position = ivec2(0, 0);
    
        if (gl_VertexID / 2 == 1)
            vertex_position.x = in_position_I_extent.z;
        if (gl_VertexID % 2 == 1)
            vertex_position.y = in_position_I_extent.w;
        
        int bitmap_width = bitmap_index_I_bitmap_width_I_bitmap_thickness_I_modifiers.y;
        
        if ((modifiers & MODIFIER_BOLD_BIT) == MODIFIER_BOLD_BIT)
            bitmap_width += bitmap_index_I_bitmap_width_I_bitmap_thickness_I_modifiers.z;
        
        ivec2 underlined_bitmap_draw_extent = ivec2((bitmap_width * scale * 8 + font_resolution - 1) / font_resolution, 9 * scale);

        fragment_position = vec2(vertex_position - ivec2());
        
        vertex_position += in_position_I_extent.xy;
        
        vec2 vertex_position_f = vec2(vertex_position);
        
        if ((modifiers & MODIFIER_ITALIC_BIT) != 0)
            vertex_position_f.x -= float(vertex_position.y - int(scale) * 7) / 2.0;
        
        if (draw_mode == 1)
            vertex_position_f += float(scale);
        
        gl_Position = screen_to_ndc * vec4(vertex_position_f, 0.0, 1.0);
    }

}