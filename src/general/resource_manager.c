#include <stdio.h>
#include <stdlib.h>

#include "stb_image_include.h"

#include "resource_manager.h"

char* parse_file(const char* filename) {

    FILE* file = fopen(filename, "rb");
    if (file == NULL) return NULL;

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc((fileSize + 1) * sizeof(char));
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead != fileSize) {
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[fileSize] = '\0';

    fclose(file);

    return buffer;
}

struct argb_image* load_png(const char* file_name) {
    int width, height, channels;
    unsigned char* img = stbi_load(file_name, &width, &height, &channels, 4);
    if (!img) {
        printf("Failed to load image %s\n", file_name);
        return NULL;
    }

    struct argb_image* image = (struct argb_image*)malloc(sizeof(struct argb_image) + width * height * sizeof(union argb_pixel));
    if(image == NULL) {
        stbi_image_free(img);
        return NULL;
    }

    image->width = width;
    image->height = height;
    image->pixels = (unsigned long long) image + sizeof(struct argb_image);

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
