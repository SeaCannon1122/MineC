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
    
    if ((draw_mode == 0 && (masks & PIXELCHAR_MASK_BACKGROUND) == 0) || (draw_mode == 1 && (masks & PIXELCHAR_MASK_SHADOW) == 0) || font_resolution_index_width.x == VULKAN_PIXELFONT_INVALID)
        fragmentColor = vec4(0.0, 0.0, 0.0, 0.0);
    else
    {
        if (draw_mode == 0)
            fragmentColor = background_color;
        else
        {
            int bit_set = 0;
            
            if ()
            
                uvec2 bitmap_pos = uvec2(fragment_position / vec2(char_dims) * vec2(font_resolution_index_width.w, font_resolution_index_width.y));

            if (font_resolution_index_width.x == 0)
            {
                if (PIXELCHAR_IF_BIT(font0_bitmaps, font_resolution_index_width.y, font_resolution_index_width.z, bitmap_pos.x, bitmap_pos.y) != 0)
                    bit_set = 1;
            }
            else if (font_resolution_index_width.x == 1)
            {
                if (PIXELCHAR_IF_BIT(font1_bitmaps, font_resolution_index_width.y, font_resolution_index_width.z, bitmap_pos.x, bitmap_pos.y) != 0)
                    bit_set = 1;
            }
            else if (font_resolution_index_width.x == 2)
            {
                if (PIXELCHAR_IF_BIT(font2_bitmaps, font_resolution_index_width.y, font_resolution_index_width.z, bitmap_pos.x, bitmap_pos.y) != 0)
                    bit_set = 1;
            }
            else if (font_resolution_index_width.x == 3)
            {
                if (PIXELCHAR_IF_BIT(font3_bitmaps, font_resolution_index_width.y, font_resolution_index_width.z, bitmap_pos.x, bitmap_pos.y) != 0)
                    bit_set = 1;
            }
                
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