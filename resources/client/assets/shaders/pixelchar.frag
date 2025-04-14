#version 450

#define PIXELCHAR_IF_BIT(ptr, resolution, index, x, y) (ptr[((x) + (y) * (resolution)) / 32 + (index) * (resolution) * (resolution) / 32] & (1 << (((x) + (y) * (resolution)) % 32)) )

uint VULKAN_PIXELFONT_INVALID = 255;

uint PIXELCHAR_MASK_UNDERLINE = 1;
uint PIXELCHAR_MASK_CURSIVE = 2;
uint PIXELCHAR_MASK_SHADOW = 4;
uint PIXELCHAR_MASK_BACKGROUND = 8;

layout(set = 0, binding = 0) readonly buffer font0 { uint font0_bitmaps[]; };
layout(set = 0, binding = 1) readonly buffer font1 { uint font1_bitmaps[]; };
layout(set = 0, binding = 2) readonly buffer font2 { uint font2_bitmaps[]; };
layout(set = 0, binding = 3) readonly buffer font3 { uint font3_bitmaps[]; };
layout(set = 0, binding = 4) readonly buffer font4 { uint font4_bitmaps[]; };
layout(set = 0, binding = 5) readonly buffer font5 { uint font5_bitmaps[]; };
layout(set = 0, binding = 6) readonly buffer font6 { uint font6_bitmaps[]; };
layout(set = 0, binding = 7) readonly buffer font7 { uint font7_bitmaps[]; };

layout(push_constant) uniform PushConstants 
{
    ivec2 screen_size;
    vec4 shadow_color_devisor;
    uint draw_mode;
};

layout(location = 0) in vec4 color;
layout(location = 1) in vec4 background_color;
layout(location = 2) flat in uvec4 font_resolution_index_width;
layout(location = 3) flat in uint masks;
layout(location = 4) in vec2 fragment_position;
layout(location = 5) flat in ivec2 char_dims;

layout (location = 0) out vec4 fragmentColor;


void main() {
    fragmentColor = vec4(0.0, 0.0, 0.0, 0.0);
    
    int res = int(font_resolution_index_width.y);
    int size = char_dims.y / 8;
    
    ivec2 fragment_position_int = ivec2(fragment_position);
    
    if (!(draw_mode == 0 && (masks & PIXELCHAR_MASK_BACKGROUND) == 0) && !(draw_mode == 1 && (masks & PIXELCHAR_MASK_SHADOW) == 0) && font_resolution_index_width.x != VULKAN_PIXELFONT_INVALID)
    {
        if (draw_mode == 0)
            fragmentColor = background_color;
        else
        {
            int bit_set = 0;
            
            if (
            fragment_position.x >= 0.0 &&
            fragment_position.y >= 0.0 &&
            fragment_position.x / float(size) < 8.0 &&
            fragment_position.y / float(size) < 8.0
            )
            {
                fragment_position_int.x = fragment_position_int.x - res / 4 * size;
                fragment_position_int.y = fragment_position_int.y - res / 4 * size;
        
                ivec2 check_coords = fragment_position_int;
        
                if (fragment_position_int.x < 0)
                    check_coords.x += 1;
                if (fragment_position_int.y < 0)
                    check_coords.y += 1;
        
                check_coords.x = check_coords.x * 2 / size + res / 2;
                check_coords.y = check_coords.y * 2 / size + res / 2;
        
                if (fragment_position_int.x < 0)
                    check_coords.x -= 1;
                if (fragment_position_int.y < 0)
                    check_coords.y -= 1;
            
                if (check_coords.x >= 0 && check_coords.y >= 0 && check_coords.x < font_resolution_index_width.y && check_coords.y < font_resolution_index_width.y)
                {

                    if (font_resolution_index_width.x == 0)
                    {
                        if (PIXELCHAR_IF_BIT(font0_bitmaps, font_resolution_index_width.y, font_resolution_index_width.z, check_coords.x, check_coords.y) != 0)
                            bit_set = 1;
                    }
                    else if (font_resolution_index_width.x == 1)
                    {
                        if (PIXELCHAR_IF_BIT(font1_bitmaps, font_resolution_index_width.y, font_resolution_index_width.z, check_coords.x, check_coords.y) != 0)
                            bit_set = 1;
                    }
                    else if (font_resolution_index_width.x == 2)
                    {
                        if (PIXELCHAR_IF_BIT(font2_bitmaps, font_resolution_index_width.y, font_resolution_index_width.z, check_coords.x, check_coords.y) != 0)
                            bit_set = 1;
                    }
                    else if (font_resolution_index_width.x == 3)
                    {
                        if (PIXELCHAR_IF_BIT(font3_bitmaps, font_resolution_index_width.y, font_resolution_index_width.z, check_coords.x, check_coords.y) != 0)
                            bit_set = 1;
                    }
                    else if (font_resolution_index_width.x == 4)
                    {
                        if (PIXELCHAR_IF_BIT(font4_bitmaps, font_resolution_index_width.y, font_resolution_index_width.z, check_coords.x, check_coords.y) != 0)
                            bit_set = 1;
                    }
                    else if (font_resolution_index_width.x == 5)
                    {
                        if (PIXELCHAR_IF_BIT(font5_bitmaps, font_resolution_index_width.y, font_resolution_index_width.z, check_coords.x, check_coords.y) != 0)
                            bit_set = 1;
                    }
                    else if (font_resolution_index_width.x == 6)
                    {
                        if (PIXELCHAR_IF_BIT(font6_bitmaps, font_resolution_index_width.y, font_resolution_index_width.z, check_coords.x, check_coords.y) != 0)
                            bit_set = 1;
                    }
                    else if (font_resolution_index_width.x == 7)
                    {
                        if (PIXELCHAR_IF_BIT(font7_bitmaps, font_resolution_index_width.y, font_resolution_index_width.z, check_coords.x, check_coords.y) != 0)
                            bit_set = 1;
                    }
                }
            }
            else if (fragment_position_int.y / size > 7 && fragment_position_int.y / size < 9 && (masks & PIXELCHAR_MASK_UNDERLINE) != 0)
                bit_set = 1;
                
            if (bit_set == 1)
            {
                if (draw_mode == 1)
                    fragmentColor = color / shadow_color_devisor;
                else
                    fragmentColor = color;
            }
        }
    }

}