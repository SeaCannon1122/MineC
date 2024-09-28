#pragma once 

struct game_chat_char {
	char value;
	char font;
	union {
		unsigned int color_value;
		struct {
			unsigned char b;
			unsigned char g;
			unsigned char r;
			unsigned char a;
		} color;
	} color;
};