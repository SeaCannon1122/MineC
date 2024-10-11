#pragma once

#include "keyvalue.h"

void* load_file(char* filename, int* size);

char* load_text_file(char* filename);

struct char_font* load_char_font(char* src);

struct key_value_map* load_key_value_map(char* src);