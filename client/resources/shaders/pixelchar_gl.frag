#version 410 core

#extension GL_ARB_shader_storage_buffer_object : enable

#define PIXELCHAR_IF_BIT(ptr, resolution, index, x, y) (ptr[((x) + (y) * (resolution)) / 32 + (index) * (resolution) * (resolution) / 32] & (1 << (((x) + (y) * (resolution)) % 32)) )

uint VULKAN_PIXELFONT_INVALID = 255;

uint PIXELCHAR_MASK_UNDERLINE = 1;
uint PIXELCHAR_MASK_CURSIVE = 2;
uint PIXELCHAR_MASK_SHADOW = 4;
uint PIXELCHAR_MASK_BACKGROUND = 8;

layout(binding = 0) readonly buffer font0 { uint font0_bitmaps[]; };
layout(binding = 1) readonly buffer font1 { uint font1_bitmaps[]; };
layout(binding = 2) readonly buffer font2 { uint font2_bitmaps[]; };
layout(binding = 3) readonly buffer font3 { uint font3_bitmaps[]; };
layout(binding = 4) readonly buffer font4 { uint font4_bitmaps[]; };
layout(binding = 5) readonly buffer font5 { uint font5_bitmaps[]; };
layout(binding = 6) readonly buffer font6 { uint font6_bitmaps[]; };
layout(binding = 7) readonly buffer font7 { uint font7_bitmaps[]; };

uniform ivec2 screen_size;
uniform vec4 shadow_color_devisor;
uniform uint draw_mode;

flat in uvec4 bitmap_index_I_masks_I_font_I_font_resolution;
flat in uint scale;
in vec2 fragment_position;
in vec4 color;
in vec4 background_color;

layout (location = 0) out vec4 fragmentColor;

void main() {
    
    uint invalid_bitmap[2];
    invalid_bitmap[0] = 0x392B293B;
    invalid_bitmap[1] = 0x494DEB00;
    
    uint bitmap_index = bitmap_index_I_masks_I_font_I_font_resolution.x;
    uint masks = bitmap_index_I_masks_I_font_I_font_resolution.y;
    uint font = bitmap_index_I_masks_I_font_I_font_resolution.z;
    int font_resolution = int(bitmap_index_I_masks_I_font_I_font_resolution.w);
    
    if ((draw_mode == 0 && (masks & PIXELCHAR_MASK_BACKGROUND) == 0) || (draw_mode == 1 && (masks & PIXELCHAR_MASK_SHADOW) == 0))
        fragmentColor = vec4(0.0, 0.0, 0.0, 0.0);
    else
    {
        if (draw_mode == 0)
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

                if (font == 0)
                {
                    if (PIXELCHAR_IF_BIT(font0_bitmaps, font_resolution, bitmap_index, check_coords.x, check_coords.y) != 0)
                        bit_set = 1;
                }
                else if (font == 1)
                {
                    if (PIXELCHAR_IF_BIT(font1_bitmaps, font_resolution, bitmap_index, check_coords.x, check_coords.y) != 0)
                        bit_set = 1;
                }
                else if (font == 2)
                {
                    if (PIXELCHAR_IF_BIT(font2_bitmaps, font_resolution, bitmap_index, check_coords.x, check_coords.y) != 0)
                        bit_set = 1;
                }
                else if (font == 3)
                {
                    if (PIXELCHAR_IF_BIT(font3_bitmaps, font_resolution, bitmap_index, check_coords.x, check_coords.y) != 0)
                        bit_set = 1;
                }
                else if (font == 4)
                {
                    if (PIXELCHAR_IF_BIT(font4_bitmaps, font_resolution, bitmap_index, check_coords.x, check_coords.y) != 0)
                        bit_set = 1;
                }
                else if (font == 5)
                {
                    if (PIXELCHAR_IF_BIT(font5_bitmaps, font_resolution, bitmap_index, check_coords.x, check_coords.y) != 0)
                        bit_set = 1;
                }
                else if (font == 6)
                {
                    if (PIXELCHAR_IF_BIT(font6_bitmaps, font_resolution, bitmap_index, check_coords.x, check_coords.y) != 0)
                        bit_set = 1;
                }
                else if (font == 7)
                {
                    if (PIXELCHAR_IF_BIT(font7_bitmaps, font_resolution, bitmap_index, check_coords.x, check_coords.y) != 0)
                        bit_set = 1;
                }
                else
                {
                    if (PIXELCHAR_IF_BIT(invalid_bitmap, 8, 0, check_coords.x, check_coords.y) != 0)
                        bit_set = 1;
                }

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

}