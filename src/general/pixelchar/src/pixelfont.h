#pragma once

#ifndef _PIXELCHAR_PIXELFONT_H
#define _PIXELCHAR_PIXELFONT_H

#include <stdint.h>

struct pixelfont {
	struct {
		uint64_t width;
		uint16_t layout[16];
	} bitmaps[0x20000];
};

struct pixelfont* pixelchar_load_font(uint8_t* src);

#endif
