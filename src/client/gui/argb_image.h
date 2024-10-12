#pragma once

#ifndef ALIGNMENTS
#define ALIGNMENTS

#define ALIGNMENT_LEFT   0
#define ALIGNMENT_RIGHT  1
#define ALIGNMENT_TOP    2
#define ALIGNMENT_BOTTOM 3
#define ALIGNMENT_MIDDLE 5

#endif // !ALIGNMENTS

#define MODE_RGB  0
#define MODE_ARGB 1

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
	union argb_pixel pixels[];
};

struct argb_image* load_argb_image_from_png(char* file_name);

int argb_image_get_rel_position(struct argb_image* image, int x, int y, int alignment_x, int alignment_y, int flip_x, int flip_y, int width, int height, int scalar, int* x_rel, int* y_rel, int mouse_x, int mouse_y);

void argb_image_draw(int mode, struct argb_image* image, int x, int y, int alignment_x, int alignment_y, int flip_x, int flip_y, unsigned int* screen, int width, int height, int scalar);