#version 450

#define PIXEL_CHAR_IF_BIT(ptr, x, y) (ptr[(x + y * 16) / 32] & (1 << ((x + y * 16) % 32)) )


#define PIXEL_CHAR_UNDERLINE_MASK  0x80000000
#define PIXEL_CHAR_CURSIVE_MASK    0x40000000
#define PIXEL_CHAR_SHADOW_MASK     0x20000000
#define PIXEL_CHAR_BACKGROUND_MASK 0x10000000
#define PIXEL_CHAR_FONT_MASK       0x0fffffff

struct char_font_entry
{
    uint width;
    uint space;
    int pixel_layout[8];
};

struct pixel_char
{
    vec4 color;
    vec4 background_color;
    uint value;
    uint space;
    uint masks;
};

struct character
{
    int size;
    vec2 start_position;
    pixel_char pixel_char_data;
};



layout(set = 0, binding = 0) readonly buffer pixel_char_data {
	character chars[];
};

layout(set = 0, binding = 1) readonly buffer font0 {
	char_font_entry char_font_entries_0[];
};
layout(set = 0, binding = 1) readonly buffer font1 {
	char_font_entry char_font_entries_1[];
};
layout(set = 0, binding = 1) readonly buffer font2 {
	char_font_entry char_font_entries_2[];
};
layout(set = 0, binding = 1) readonly buffer font3 {
	char_font_entry char_font_entries_3[];
};

layout(push_constant) uniform PushConstants {
	int screen_width;
	int screen_height;
	int draw_mode;
};



layout (location = 0) in vec2 f_fragment_position;
layout (location = 1) in float f_char_index;

layout (location = 0) out vec4 fragmentColor;



void main() {

    int char_index = int(f_char_index);
    
    int size = chars[char_index].size;
    
    ivec2 fragment_position = ivec2(f_fragment_position.x , f_fragment_position.y);
    
    fragmentColor = vec4(0.0, 0.0, 0.0, 0.0);
    
    if (draw_mode == 0)
    {
        
        if ((chars[char_index].pixel_char_data.masks & PIXEL_CHAR_BACKGROUND_MASK) != 0)
            fragmentColor = vec4(chars[char_index].pixel_char_data.background_color);

    }
    else if (draw_mode == 1)
    {
        if ((chars[char_index].pixel_char_data.masks & PIXEL_CHAR_SHADOW_MASK) != 0)
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
        
                if (PIXEL_CHAR_IF_BIT(char_font_entries_0[chars[char_index].pixel_char_data.value].pixel_layout, check_coords.x, check_coords.y) != 0)
                    fragmentColor = vec4(chars[char_index].pixel_char_data.color.x / 3.0, chars[char_index].pixel_char_data.color.y / 3.0, chars[char_index].pixel_char_data.color.z / 3.0, chars[char_index].pixel_char_data.color.z);
            }
            if (fragment_position.y / size > 7 && fragment_position.y / size < 9)
            {
                if ((chars[char_index].pixel_char_data.masks & PIXEL_CHAR_UNDERLINE_MASK) != 0)
                    fragmentColor = vec4(chars[char_index].pixel_char_data.color.x / 3.0, chars[char_index].pixel_char_data.color.y / 3.0, chars[char_index].pixel_char_data.color.z / 3.0, chars[char_index].pixel_char_data.color.z);
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
        
            if (PIXEL_CHAR_IF_BIT(char_font_entries_0[chars[char_index].pixel_char_data.value].pixel_layout, check_coords.x, check_coords.y) != 0)
                fragmentColor = chars[char_index].pixel_char_data.color;
        }
        if (fragment_position.y / size > 7 && fragment_position.y / size < 9)
        {
            if ((chars[char_index].pixel_char_data.masks & PIXEL_CHAR_UNDERLINE_MASK) != 0)
                fragmentColor = chars[char_index].pixel_char_data.color;
        }
        
    }


}