#version 450

const int MODIFIER_BOLD_BIT = 1;
const int MODIFIER_ITALIC_BIT = 2;
const int MODIFIER_UNDERLINE_BIT = 4;
const int MODIFIER_STRIKETHROUGH_BIT = 8;
const int MODIFIER_SHADOW_BIT = 16;
const int MODIFIER_BACKGROUND_BIT = 32;

layout(set = 0, binding = 0) readonly buffer bitmaps_buffer { uint bitmaps[]; };

layout(push_constant) uniform PushConstants 
{
    mat4 screen_to_ndc;
    vec4 shadow_color_devisor;
    int font_resolution;
    int bitmap_count;
    uint draw_mode;
};

layout(location = 0) flat in ivec4 bitmap_index_I_bitmap_thickness_I_modifiers_I_scale;
layout(location = 1) in vec2 fragment_position;
layout(location = 2) in vec4 color;
layout(location = 3) in vec4 background_color;

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
            
        if (pixel_position.y >= (scale * 7 + 1) / 2 && pixel_position.y < (scale * 7 + 1) / 2 + scale && (modifiers & MODIFIER_STRIKETHROUGH_BIT) != 0)
            bit_set = 1;
        else if (
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
            
            for (int check_x = check_coords.x; check_x >= 0 && check_x >= check_coords.x - ((modifiers & MODIFIER_STRIKETHROUGH_BIT) != 0 ? bitmap_thickness : 0); check_x--)
            {
                if (check_x < font_resolution)
                {
                    uint bitmap_bit_index = check_x + check_coords.y * font_resolution;
            
                    if ((bitmaps[bitmap_bit_index / 32 + bitmap_index * font_resolution * font_resolution / 32] & (1 << (bitmap_bit_index % 32))) != 0)
                        bit_set = 1;
                }
            }
            
        }
        else if (pixel_position.y / scale > 7 && pixel_position.y / scale < 9 && (modifiers & MODIFIER_UNDERLINE_BIT) != 0)
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