#include "argb_image.h"

#include <STB_IMAGE/stb_image.h>

#define __ARGB_IMAGE_CLAMP(value, mi, ma) (value > ma ? ma : (value < mi ? mi : value))

struct argb_image* load_argb_image_from_png(char* file_name) {
    int width, height, channels;
    unsigned char* img = stbi_load(file_name, &width, &height, &channels, 4);
    if (!img) return NULL;


    struct argb_image* image = (struct argb_image*)malloc(sizeof(struct argb_image) + width * height * sizeof(union argb_pixel));
    if (image == NULL) {
        stbi_image_free(img);
        return NULL;
    }

    image->width = width;
    image->height = height;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char* px = &img[(y * width + x) * 4];
            image->pixels[y * width + x].color.r = px[0];
            image->pixels[y * width + x].color.g = px[1];
            image->pixels[y * width + x].color.b = px[2];
            image->pixels[y * width + x].color.a = px[3];
        }
    }

    stbi_image_free(img);

    return image;
}

int argb_image_get_rel_position(struct argb_image* image, int x, int y, int alignment_x, int alignment_y, int flip_x, int flip_y, int width, int height, int scalar, int* x_rel, int* y_rel, int mouse_x, int mouse_y) {

    int x_min = x - (alignment_x == ALIGNMENT_LEFT ? 0 : (alignment_x == ALIGNMENT_RIGHT ? image->width * scalar : image->width * scalar / 2));
    int y_min = y - (alignment_y == ALIGNMENT_TOP ? 0 : (alignment_y == ALIGNMENT_BOTTOM ? image->height * scalar : image->height * scalar / 2));

    int x_min_screen = __ARGB_IMAGE_CLAMP(x_min, 0, width);
    int y_min_screen = __ARGB_IMAGE_CLAMP(y_min, 0, height);
    int x_max_screen = __ARGB_IMAGE_CLAMP(x_min + scalar * image->width, 0, width);
    int y_max_screen = __ARGB_IMAGE_CLAMP(y_min + scalar * image->height, 0, height);

    if (mouse_x >= x_min_screen && mouse_x < x_max_screen && mouse_y >= y_min_screen && mouse_y < y_max_screen) {
        *x_rel = x_min_screen - mouse_x;
        *y_rel = y_min_screen - mouse_y;
        int x_cord = (flip_x ? ((image->width - 1 - (x_min - *x_rel) / scalar) % image->width + image->width) % image->width : (((*x_rel - x_min) / scalar) % image->width + image->width) % image->width);
        int y_cord = (flip_y ? ((image->height - 1 - (y_min - *y_rel) / scalar) % image->height + image->height) % image->height : (((*y_rel - y_min) / scalar) % image->height + image->height) % image->height);

        return x_cord + image->width * y_cord;
    }
    else {
        *x_rel = -1;
        *y_rel = -1;
        return -1;
    }

}

void argb_image_draw(int mode, struct argb_image* image, int x, int y, int alignment_x, int alignment_y, int flip_x, int flip_y, unsigned int* screen, int width, int height, int scalar) {

    int x_min = x - (alignment_x == ALIGNMENT_LEFT ? 0 : (alignment_x == ALIGNMENT_RIGHT ? image->width * scalar : image->width * scalar / 2));
    int y_min = y - (alignment_y == ALIGNMENT_TOP ? 0 : (alignment_y == ALIGNMENT_BOTTOM ? image->height * scalar : image->height * scalar / 2));

    int x_min_screen = __ARGB_IMAGE_CLAMP(x_min, 0, width);
    int y_min_screen = __ARGB_IMAGE_CLAMP(y_min, 0, height);
    int x_max_screen = __ARGB_IMAGE_CLAMP(x_min + scalar * image->width, 0, width);
    int y_max_screen = __ARGB_IMAGE_CLAMP(y_min + scalar * image->height, 0, height);

    if (mode = MODE_ARGB) {

        for (int i = x_min_screen; i < x_max_screen; i++) {
            for (int j = y_min_screen; j < y_max_screen; j++) {

                union argb_pixel top;
                int x_cord = (flip_x ? ((image->width - 1 - (x_min - i) / scalar) % image->width + image->width) % image->width : (((i - x_min) / scalar) % image->width + image->width) % image->width);
                int y_cord = (flip_y ? ((image->height - 1 - (y_min - j) / scalar) % image->height + image->height) % image->height : (((j - y_min) / scalar) % image->height + image->height) % image->height);

                top.color_value = image->pixels[x_cord + image->width * y_cord].color_value;
                union argb_pixel bottom;
                bottom.color_value = screen[i + j * width];

                ((union argb_pixel*)screen)[i + j * width].color.a = (unsigned char)(((unsigned int)top.color.a * (unsigned int)top.color.a + (255 - (unsigned int)top.color.a) * (unsigned int)bottom.color.a) / 255);
                ((union argb_pixel*)screen)[i + j * width].color.r = (unsigned char)(((unsigned int)top.color.r * (unsigned int)top.color.a + (255 - (unsigned int)top.color.a) * (unsigned int)bottom.color.r) / 255);
                ((union argb_pixel*)screen)[i + j * width].color.g = (unsigned char)(((unsigned int)top.color.g * (unsigned int)top.color.a + (255 - (unsigned int)top.color.a) * (unsigned int)bottom.color.g) / 255);
                ((union argb_pixel*)screen)[i + j * width].color.b = (unsigned char)(((unsigned int)top.color.b * (unsigned int)top.color.a + (255 - (unsigned int)top.color.a) * (unsigned int)bottom.color.b) / 255);

            }
        }
    }

    else {
        for (int i = x_min_screen; i < x_max_screen; i++) {
            for (int j = y_min_screen; j < y_max_screen; j++) {

                int x_cord = (flip_x ? ((image->width - 1 - (x_min - i) / scalar) % image->width + image->width) % image->width : (((i - x_min) / scalar) % image->width + image->width) % image->width);
                int y_cord = (flip_y ? ((image->height - 1 - (y_min - j) / scalar) % image->height + image->height) % image->height : (((j - y_min) / scalar) % image->height + image->height) % image->height);
                
                screen[i + j * width] = image->pixels[x_cord + image->width * y_cord].color_value;
            }
        }
    }

}