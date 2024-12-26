#version 450

#define PIXEL_CHAR_IF_BIT(ptr, x, y) (ptr[(x + y * 16) / 32] & (1 << ((x + y * 16) % 32)) )

#define PIXEL_CHAR_SHADOW_DIVISOR 3.0

#define PIXEL_CHAR_UNDERLINE_MASK  0x8000
#define PIXEL_CHAR_CURSIVE_MASK    0x4000
#define PIXEL_CHAR_SHADOW_MASK     0x2000
#define PIXEL_CHAR_BACKGROUND_MASK 0x1000
#define PIXEL_CHAR_FONT_MASK       0x00ff

struct char_font_entry
{
    uint width;
    uint space;
    int pixel_layout[8];
};

layout(set = 0, binding = 0) readonly buffer font0 { char_font_entry char_font_entries_0[]; };
layout(set = 0, binding = 1) readonly buffer font1 { char_font_entry char_font_entries_1[]; };
layout(set = 0, binding = 2) readonly buffer font2 { char_font_entry char_font_entries_2[]; };
layout(set = 0, binding = 3) readonly buffer font3 { char_font_entry char_font_entries_3[]; };

layout(push_constant) uniform PushConstants {
	int screen_width;
	int screen_height;
	uint draw_mode;
};

layout(location = 0) in vec4 color;
layout(location = 1) in vec4 background_color;
layout(location = 2) flat in uint value;
layout(location = 3) in vec2 f_fragment_position;
layout(location = 4) flat in uint masks;
layout(location = 5) flat in int size;

layout (location = 0) out vec4 fragmentColor;


void main() {
    
    char_font_entry font_entry;
	
    if ((masks & PIXEL_CHAR_FONT_MASK) == 0) font_entry = char_font_entries_0[value];
	else if ((masks & PIXEL_CHAR_FONT_MASK) == 1) font_entry = char_font_entries_1[value];
	else if ((masks & PIXEL_CHAR_FONT_MASK) == 2) font_entry = char_font_entries_2[value];
	else if ((masks & PIXEL_CHAR_FONT_MASK) == 3) font_entry = char_font_entries_3[value];
    
    ivec2 fragment_position = ivec2(f_fragment_position);
    
    fragmentColor = vec4(0.0, 0.0, 0.0, 0.0);
    
    if (draw_mode == 0)
    { 
        if ((masks & PIXEL_CHAR_BACKGROUND_MASK) != 0)
            fragmentColor = background_color;

    }
    else if (draw_mode == 1)
    {
        if ((masks & PIXEL_CHAR_SHADOW_MASK) != 0)
        {
            if (
            f_fragment_position.x >= 0.0 &&
            f_fragment_position.y >= 0.0 &&
            f_fragment_position.x / float(size) < 8.0 &&
            f_fragment_position.y / float(size) < 8.0
            )
            {
        
                fragment_position.x = fragment_position.x - 4 * size;
                fragment_position.y = fragment_position.y - 4 * size;
        
                ivec2 check_coords = fragment_position;
        
                if (fragment_position.x < 0)
                    check_coords.x += 1;
                if (fragment_position.y < 0)
                    check_coords.y += 1;
        
                check_coords.x = check_coords.x * 2 / size + 8;
                check_coords.y = check_coords.y * 2 / size + 8;
        
                if (fragment_position.x < 0)
                    check_coords.x -= 1;
                if (fragment_position.y < 0)
                    check_coords.y -= 1;
        
                if (PIXEL_CHAR_IF_BIT(font_entry.pixel_layout, check_coords.x, check_coords.y) != 0)
                    fragmentColor = vec4(
                        color.xyz / PIXEL_CHAR_SHADOW_DIVISOR, 
                        color.w
                    );
            }
            if (fragment_position.y / size > 7 && fragment_position.y / size < 9)
            {
                if ((masks & PIXEL_CHAR_UNDERLINE_MASK) != 0)
                    fragmentColor = vec4(
                        color.xyz / PIXEL_CHAR_SHADOW_DIVISOR, 
                        color.w
                    );
            }
        }

    }
    else 
    {
    
        if (
            f_fragment_position.x >= 0.0 &&
            f_fragment_position.y >= 0.0 &&
            f_fragment_position.x / float(size) < 8.0 &&
            f_fragment_position.y / float(size) < 8.0
        )
        {
        
            fragment_position.x = fragment_position.x - 4 * size;
            fragment_position.y = fragment_position.y - 4 * size;
        
            ivec2 check_coords = fragment_position;
        
            if (fragment_position.x < 0)
                check_coords.x += 1;
            if (fragment_position.y < 0)
                check_coords.y += 1;
        
            check_coords.x = check_coords.x * 2 / size + 8;
            check_coords.y = check_coords.y * 2 / size + 8;
        
            if (fragment_position.x < 0)
                check_coords.x -= 1;
            if (fragment_position.y < 0)
                check_coords.y -= 1;
        
            if (PIXEL_CHAR_IF_BIT(font_entry.pixel_layout, check_coords.x, check_coords.y) != 0)
                fragmentColor = color;
        }
        if (fragment_position.y / size > 7 && fragment_position.y / size < 9)
        {
            if ((masks & PIXEL_CHAR_UNDERLINE_MASK) != 0)
                fragmentColor = color;
        }
        
    }


}