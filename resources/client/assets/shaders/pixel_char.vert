#version 450

#define PIXEL_CHAR_UNDERLINE_MASK  0x8000
#define PIXEL_CHAR_CURSIVE_MASK    0x4000
#define PIXEL_CHAR_SHADOW_MASK     0x2000
#define PIXEL_CHAR_BACKGROUND_MASK 0x1000
#define PIXEL_CHAR_FONT_MASK       0x00ff

struct char_font_entry {
		uint width;
		uint space;
		uint pixel_layout[8];
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

layout(location = 0) in vec4 in_color;
layout(location = 1) in vec4 in_background_color;
layout(location = 2) in uint in_value;
layout(location = 3) in ivec2 in_position;
layout(location = 4) in uint in_masks;
layout(location = 5) in int in_size;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 background_color;
layout(location = 2) flat out uint value;
layout(location = 3) out vec2 f_fragment_position;
layout(location = 4) flat out uint masks;
layout(location = 5) flat out int size;



void main() {

    color = in_color;
    background_color = in_background_color;
    value = in_value;
    masks = in_masks;
    size = in_size;
	
    char_font_entry font_entry;
	
    if ((masks & PIXEL_CHAR_FONT_MASK) == 0) font_entry = char_font_entries_0[value];
	else if ((masks & PIXEL_CHAR_FONT_MASK) == 1) font_entry = char_font_entries_1[value];
	else if ((masks & PIXEL_CHAR_FONT_MASK) == 2) font_entry = char_font_entries_2[value];
	else if ((masks & PIXEL_CHAR_FONT_MASK) == 3) font_entry = char_font_entries_3[value];
	
    ivec2 vertex_multiplier;
	
	if (gl_VertexIndex % 6 == 0) vertex_multiplier = ivec2(0, 1);
	else if (gl_VertexIndex % 6 == 1) vertex_multiplier = ivec2(1, 0);
	else if (gl_VertexIndex % 6 == 2) vertex_multiplier = ivec2(0, 0);
	else if (gl_VertexIndex % 6 == 3) vertex_multiplier = ivec2(0, 1);
	else if (gl_VertexIndex % 6 == 4) vertex_multiplier = ivec2(1, 1);
	else if (gl_VertexIndex % 6 == 5) vertex_multiplier = ivec2(1, 0);

    vec2 vertex_position = vec2(ivec2(
		-size / 2 + vertex_multiplier.x * (int(font_entry.width + 3) / 2 * size),
		-size / 2 + vertex_multiplier.y * 10 * size
	));
	
    f_fragment_position = vertex_position;
	
    if ((masks & PIXEL_CHAR_CURSIVE_MASK) != 0) 
	{
        vertex_position.x = vertex_position.x - (vertex_position.y - 7.0 * float(size)) / 2.0;
    }
	
    if (draw_mode == 1)
    {
        vertex_position.x += float((3 * size) / 5 + 1);
        vertex_position.y += float((3 * size) / 5 + 1);
    }
	
    gl_Position = vec4(
		-1.0 + 2.0 * (float(in_position.x) + vertex_position.x) / float(screen_width),
		-1.0 + 2.0 * (float(in_position.y) + vertex_position.y) / float(screen_height),
		1.0,
		1.0
	);

}