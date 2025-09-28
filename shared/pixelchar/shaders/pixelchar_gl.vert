#version 410 core

const int MODIFIER_BOLD_BIT = 1;
const int MODIFIER_ITALIC_BIT = 2;
const int MODIFIER_UNDERLINE_BIT = 4;
const int MODIFIER_STRIKETHROUGH_BIT = 8;
const int MODIFIER_SHADOW_BIT = 16;
const int MODIFIER_BACKGROUND_BIT = 32;

uniform int font_resolution;
uniform int bitmap_count;
uniform mat4 screen_to_ndc;
uniform int draw_mode;

layout(location = 0) in ivec4 in_position_I_width_I_scale;
layout(location = 1) in ivec4 in_bitmap_index_I_bitmap_width_I_bitmap_thickness_I_modifiers;
layout(location = 2) in vec4 in_color;
layout(location = 3) in vec4 in_background_color;

flat out ivec4 bitmap_index_I_bitmap_thickness_I_modifiers_I_scale;
out vec2 fragment_position;
out vec4 color;
out vec4 background_color;

void main()
{
    if (in_bitmap_index_I_bitmap_width_I_bitmap_thickness_I_modifiers.x < bitmap_count)
        bitmap_index_I_bitmap_thickness_I_modifiers_I_scale.x = in_bitmap_index_I_bitmap_width_I_bitmap_thickness_I_modifiers.x;
    else 
        bitmap_index_I_bitmap_thickness_I_modifiers_I_scale.x = 0;
    bitmap_index_I_bitmap_thickness_I_modifiers_I_scale.y = in_bitmap_index_I_bitmap_width_I_bitmap_thickness_I_modifiers.z;
    bitmap_index_I_bitmap_thickness_I_modifiers_I_scale.z = in_bitmap_index_I_bitmap_width_I_bitmap_thickness_I_modifiers.w;
    bitmap_index_I_bitmap_thickness_I_modifiers_I_scale.w = in_position_I_width_I_scale.w;
    color = in_color;
    background_color = in_background_color;
    
    int bitmap_width = in_bitmap_index_I_bitmap_width_I_bitmap_thickness_I_modifiers.y;
    int bitmap_thickness = in_bitmap_index_I_bitmap_width_I_bitmap_thickness_I_modifiers.z;
    int modifiers = in_bitmap_index_I_bitmap_width_I_bitmap_thickness_I_modifiers.w;
    int width = in_position_I_width_I_scale.z;
    int scale = in_position_I_width_I_scale.w;
    
    if ((draw_mode == 0 && (modifiers & MODIFIER_BACKGROUND_BIT) == 0) || (draw_mode == 1 && (modifiers & MODIFIER_SHADOW_BIT) == 0))
        gl_Position = vec4(2.0, 2.0, 0.0, 1.0);
    else
    {
        ivec2 vertex_position = ivec2(0, -((scale + 1) / 2));
    
        if (gl_VertexID / 2 == 1)
            vertex_position.x += width;
        if (gl_VertexID % 2 == 1)
            vertex_position.y += scale * 10;
        
        if ((modifiers & MODIFIER_BOLD_BIT) == MODIFIER_BOLD_BIT)
            bitmap_width += bitmap_thickness;
        
        fragment_position = vec2(vertex_position - ivec2((width - bitmap_width * scale * 8 / font_resolution) / 2, 0));
        
        vec2 vertex_position_f = vec2(vertex_position);
        
        if ((modifiers & MODIFIER_ITALIC_BIT) != 0)
            vertex_position_f.x -= float(vertex_position.y - int(scale) * 7) / 2.0;
        
        vertex_position_f += vec2(in_position_I_width_I_scale.xy);
        vertex_position_f.y -= float(scale * 4);
        
        if (draw_mode == 1)
            vertex_position_f += float(scale);
        
        gl_Position = screen_to_ndc * vec4(vertex_position_f, 0.0, 1.0);
    }

}