#pragma once

union argb_pixel {
	unsigned int color_value;
	struct {
		unsigned char b;
		unsigned char g;
		unsigned char r;
		unsigned char a;
	} color;
};

struct argb_image {
	int width;
	int height;
	union argb_pixel* pixels;
};