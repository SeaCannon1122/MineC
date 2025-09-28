#version 410 core

#extension GL_ARB_shader_storage_buffer_object : enable

#define PIXELCHAR_IF_BIT(ptr, resolution, index, x, y) 

const int MODIFIER_BOLD_BIT = 1;
const int MODIFIER_ITALIC_BIT = 2;
const int MODIFIER_UNDERLINE_BIT = 4;
const int MODIFIER_STRIKETHROUGH_BIT = 8;
const int MODIFIER_SHADOW_BIT = 16;
const int MODIFIER_BACKGROUND_BIT = 32;

layout(binding = 0) readonly buffer bitmaps_buffer { uint bitmaps[]; };

uniform int font_resolution;
uniform vec4 shadow_color_devisor;
uniform int draw_mode;

flat in ivec4 bitmap_index_I_bitmap_thickness_I_modifiers_I_scale;
in vec2 fragment_position;
in vec4 color;
in vec4 background_color;

layout (location = 0) out vec4 fragmentColor;

void main() {
    
    int bitmap_index = bitmap_index_I_bitmap_thickness_I_modifiers_I_scale.x;
    int bitmap_thickness = bitmap_index_I_bitmap_thickness_I_modifiers_I_scale.y;
    int modifiers = bitmap_index_I_bitmap_thickness_I_modifiers_I_scale.z;
    int scale = bitmap_index_I_bitmap_thickness_I_modifiers_I_scale.w;

    if ((draw_mode == 0 && (modifiers & MODIFIER_BACKGROUND_BIT) == 0) || (draw_mode == 1 && (modifiers & MODIFIER_SHADOW_BIT) == 0))
        discard;
    else if (draw_mode == 0)
        fragmentColor = background_color;
    else
    {
        int bit_set = 0;
         
        ivec2 pixel_position = ivec2(fragment_position);
            
        if (
            fragment_position.x >= 0.0 &&
            fragment_position.y >= 0.0 &&
            fragment_position.y / scale < 8.0
        )
        {
            pixel_position -= scale * 4;
        
            ivec2 check_coords = pixel_position;
        
            if (pixel_position.x < 0)
                check_coords.x += 1;
            if (pixel_position.y < 0)
                check_coords.y += 1;
        
            check_coords = check_coords * font_resolution / (scale * 8) + font_resolution / 2;

            if (pixel_position.x < 0)
                check_coords.x -= 1;
            if (pixel_position.y < 0)
                check_coords.y -= 1;
            
            if (check_coords.x < 0 || check_coords.x >= font_resolution || check_coords.y < 0 || check_coords.y >= font_resolution)
            {
                return;
            }
            
            uint bitmap_bit_index = check_coords.x + check_coords.y * font_resolution;
            
            if ((bitmaps[bitmap_bit_index / 32 + bitmap_index * font_resolution * font_resolution / 32] & (1 << (bitmap_bit_index % 32))) != 0)
                bit_set = 1;
        }
        else if (pixel_position.y / scale > 7 && pixel_position.y / scale < 9)
            bit_set = 1;
        
        if (bit_set == 1)
        {
            if (draw_mode == 1)
                fragmentColor = color / shadow_color_devisor;
            else
                fragmentColor = color;
        }
        else
            fragmentColor = vec4(0.0, 0.0, 0.0, 0.0);
    }
            
}