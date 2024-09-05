#pragma once

#include "argb_image.h"
#include "client/gui/char_font.h"
#include "keyvalue.h"

void* load_file(char* filename, int* size);

char* load_text_file(char* filename);

struct argb_image* load_argb_image_from_png(char* file_name);

struct char_font* load_char_font(char* src);

struct key_value_map* load_key_value_map(char* src);