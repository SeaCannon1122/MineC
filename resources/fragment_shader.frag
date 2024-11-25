#version 450

#define PIXEL_CHAR_IF_BIT(ptr, x, y) (ptr[(x + y * 16) / 32] & (1 << ((x + y * 16) % 32)) )


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
    int pixel_layout[PIXEL_FONT_RESOLUTION * PIXEL_FONT_RESOLUTION / 32];
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

layout (location = 0) in vec2 char_vertex_position;
layout (location = 1) in float f_char_index;

layout (location = 0) out vec4 fragmentColor;

void main() {

    uint char_index = uint(f_char_index);
    uvec2 fragment_position = uvec2(char_vertex_position);
    
    
    if (uint(char_vertex_position.y) < 8 * chars[char_index].size) {
        
        if (PIXEL_CHAR_IF_BIT(char_font_entries[chars[char_index].pixel_char_data.value].pixel_layout, uint(2 * fragment_position.x / chars[char_index].size), uint(2 * fragment_position.y / chars[char_index].size)) != 0)
            fragmentColor = chars[char_index].pixel_char_data.color;
        
        else fragmentColor = vec4(0.0, 0.0, 0.0, 0.0);
    }
    
    else {
        
        if ((chars[char_index].pixel_char_data.masks & PIXEL_CHAR_UNDERLINE_MASK) != 0)
            fragmentColor = chars[char_index].pixel_char_data.color;
        
        else fragmentColor = vec4(0.0, 0.0, 0.0, 0.0);
        
    }
    
    
    
    
    
    

}