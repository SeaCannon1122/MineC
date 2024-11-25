#version 450

#define PIXEL_FONT_RESOLUTION 16

#define PIXEL_CHAR_UNDERLINE_MASK  0x80000000
#define PIXEL_CHAR_CURSIVE_MASK    0x40000000
#define PIXEL_CHAR_SHADOW_MASK     0x20000000
#define PIXEL_CHAR_BACKGROUND_MASK 0x10000000
#define PIXEL_CHAR_FONT_MASK       0x0fffffff

struct char_font_entry
{
    uint width;
    uint space;
    uint pixel_layout[PIXEL_FONT_RESOLUTION * PIXEL_FONT_RESOLUTION / 32];
};

struct pixel_char
{
    vec4 color;
    vec4 background_color;
    uint value;
    uint masks;
};

struct character
{
    uint size;
    vec2 start_position;
    pixel_char pixel_char_data;
};

layout(set = 0, binding = 0) readonly buffer pixel_char_data {
    
vec2 screen_size;
character chars[];
	
};

layout(set = 0, binding = 1) readonly buffer font {
    
char_font_entry char_font_entries[];
	
};

layout(location = 0) in vec2 uv;
layout (location = 1) in flat int char_index;

layout(location = 0) out vec4 fragmentColor;

void main() {

    fragmentColor = chars[char_index].pixel_char_data.color;

}