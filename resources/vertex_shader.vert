#version 450

#define PIXEL_CHAR_UNDERLINE_MASK  0x80000000
#define PIXEL_CHAR_CURSIVE_MASK    0x40000000
#define PIXEL_CHAR_SHADOW_MASK     0x20000000
#define PIXEL_CHAR_BACKGROUND_MASK 0x10000000
#define PIXEL_CHAR_FONT_MASK       0x0fffffff

struct char_font_entry {
		int width;
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

struct character {
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



layout (location = 0) out vec2 f_fragment_position;
layout (location = 1) out float f_char_index;




void main() {

	int char_index = gl_VertexIndex / 6;

    f_char_index = float(char_index);
	
    int size = chars[char_index].size;
	
    ivec2 vertex_multiplier;
	
	if (gl_VertexIndex % 6 == 0) vertex_multiplier = ivec2(0, 1);
	else if (gl_VertexIndex % 6 == 1) vertex_multiplier = ivec2(1, 0);
	else if (gl_VertexIndex % 6 == 2) vertex_multiplier = ivec2(0, 0);
	else if (gl_VertexIndex % 6 == 3) vertex_multiplier = ivec2(0, 1);
	else if (gl_VertexIndex % 6 == 4) vertex_multiplier = ivec2(1, 1);
	else if (gl_VertexIndex % 6 == 5) vertex_multiplier = ivec2(1, 0);

    vec2 vertex_position = vec2(ivec2(
		-size / 2 + vertex_multiplier.x * ((char_font_entries_0[chars[char_index].pixel_char_data.value].width + 3) / 2 * size),
		-size / 2 + vertex_multiplier.y * 10 * size
	));
	
    f_fragment_position = vertex_position;
	
    if ((chars[char_index].pixel_char_data.masks & PIXEL_CHAR_CURSIVE_MASK) != 0) 
	{
        vertex_position.x = vertex_position.x - (vertex_position.y - 7.0 * float(size)) / 2.0;

    }
	
    if (draw_mode == 1)
    {
        vertex_position.x += float((3 * size) / 5 + 1);
        vertex_position.y += float((3 * size) / 5 + 1);
    }
	
    gl_Position = vec4(
		-1.0 + 2.0 * (chars[char_index].start_position.x + vertex_position.x) / float(screen_width),
		-1.0 + 2.0 * (chars[char_index].start_position.y + vertex_position.y) / float(screen_height),
		1.0,
		1.0
	);

}