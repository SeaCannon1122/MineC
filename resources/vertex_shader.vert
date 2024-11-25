#version 450

#define PIXEL_FONT_RESOLUTION 16

#define PIXEL_CHAR_UNDERLINE_MASK  0x80000000
#define PIXEL_CHAR_CURSIVE_MASK    0x40000000
#define PIXEL_CHAR_SHADOW_MASK     0x20000000
#define PIXEL_CHAR_BACKGROUND_MASK 0x10000000
#define PIXEL_CHAR_FONT_MASK       0x0fffffff

struct char_font_entry {
		uint width;
		uint width_space;
		int pixel_layout[PIXEL_FONT_RESOLUTION * PIXEL_FONT_RESOLUTION / 32];
};

struct pixel_char
{
    vec4 color;
    vec4 background_color;
    uint value;
    uint masks;
};

struct character {
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

layout (location = 0) out vec2 char_vertex_position;
layout (location = 1) out flat int char_index;

void main() {

	char_index = gl_VertexIndex / 6;

	if (gl_VertexIndex % 6 == 0) char_vertex_position = vec2(0.0, 1.0);
	else if (gl_VertexIndex % 6 == 1) char_vertex_position = vec2(1.0, 0.0);
	else if (gl_VertexIndex % 6 == 2) char_vertex_position = vec2(0.0, 0.0);
	else if (gl_VertexIndex % 6 == 3) char_vertex_position = vec2(0.0, 1.0);
	else if (gl_VertexIndex % 6 == 4) char_vertex_position = vec2(1.0, 1.0);
	else if (gl_VertexIndex % 6 == 5) char_vertex_position = vec2(1.0, 0.0);

    gl_Position = vec4(
		-1.0 + 2.0 * (chars[char_index].start_position.x + char_vertex_position.x * float(chars[char_index].size * char_font_entries[chars[char_index].pixel_char_data.value].width / 2)) / screen_size.x,
		-1.0 + 2.0 * (chars[char_index].start_position.y + char_vertex_position.y * float(chars[char_index].size * 8)) / screen_size.y,
		1.0,
		1.0
	);

}