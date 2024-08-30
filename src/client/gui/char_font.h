#pragma once 

enum alignment {
	ALIGNMENT_LEFT = 0,
	ALIGNMENT_RIGHT = 1,
	ALIGNMENT_TOP = 2,
	ALIGNMENT_BOTTOM = 3,
	ALIGNMENT_MIDDLE = 4,
};

struct char_font {
	struct {
		long long width;
		long long layout;
	} char_font_entries[256];
};

struct gui_character {
	unsigned int color;
	struct char_font* font;
	int size;
	char value;
};

struct char_font* load_char_font(char* src);

void print_char(struct char_font* font, char c, int font_width, unsigned int color, int x, int y, unsigned int* screen, int width, int height);

void print_string(struct char_font* font, char* str, int text_size, unsigned int color, int x, int y, unsigned int* screen, int width, int height);

void drop_font(struct char_font* font);

struct gui_character* convert_string_to_gui_string(struct char_font* font, char* str);

void print_gui_string(struct gui_character* str, int scale, int x, int y, char alignment, unsigned int* screen, int width, int height);

int gui_text_width(struct gui_character* str);