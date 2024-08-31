#pragma once

#include "argb_image.h"

char* parse_file(const char* filename);

struct argb_image* load_png(const char* file_name);