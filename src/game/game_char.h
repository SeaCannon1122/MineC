#pragma once 

#ifndef GAME_CHAR_H
#define GAME_CHAR_H

#include "general/pixel_char/pixel_char.h"

struct game_char {
	uint8_t color[4];
	uint8_t background_color[4];
	uint32_t value;
	uint16_t masks;
};

#endif // !GAME_CHAR_H