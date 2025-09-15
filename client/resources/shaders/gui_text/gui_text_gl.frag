#version 410 core

#extension GL_ARB_shader_storage_buffer_object : enable

#define PIXELCHAR_IF_BIT(ptr, resolution, index, x, y) (ptr[((x) + (y) * (resolution)) / 32 + (index) * (resolution) * (resolution) / 32] & (1 << (((x) + (y) * (resolution)) % 32)) )

const int MODIFIER_BOLD_BIT = 1;
const int MODIFIER_ITALIC_BIT = 2;
const int MODIFIER_UNDERLINE_BIT = 4;
const int MODIFIER_STRIKETHROUGH_BIT = 8;
const int MODIFIER_SHADOW_BIT = 16;
const int MODIFIER_BACKGROUND_BIT = 32;

layout(binding = 0) readonly buffer bitmaps_buffer { uint bitmaps[]; };

uniform int scale;
uniform int font_resolution;
uniform vec4 shadow_color_devisor;
uniform uint draw_mode;

flat in ivec3 bitmap_index_I_bitmap_width_I_modifiers;
in vec2 fragment_position;
in vec4 color;
in vec4 background_color;


layout (location = 0) out vec4 fragmentColor;

void main() {
    
    int bitmap_index = bitmap_index_I_bitmap_width_I_modifiers.x;
    int bitmap_width = bitmap_index_I_bitmap_width_I_modifiers.x;
    int modifiers = bitmap_index_I_bitmap_width_I_modifiers.z;

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
            fragment_position.x / float(scale) < 8.0 &&
            fragment_position.y / float(scale) < 8.0
        )
        {
            pixel_position -= int(scale) * 4;
        
            ivec2 check_coords = pixel_position;
        
            if (pixel_position.x < 0)
                check_coords.x += 1;
            if (pixel_position.y < 0)
                check_coords.y += 1;
        
            check_coords = check_coords * font_resolution / (int(scale) * 8) + font_resolution / 2;

            if (pixel_position.x < 0)
                check_coords.x -= 1;
            if (pixel_position.y < 0)
                check_coords.y -= 1;

            if (PIXELCHAR_IF_BIT(bitmaps, font_resolution, bitmap_index, check_coords.x, check_coords.y) != 0)
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