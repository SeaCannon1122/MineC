#pragma once

#define parse_string_into_gui_string(src, gui_str) { char psigs[] = src; int psigs_i = 0; for(; psigs[psigs_i] != '\0'; psigs_i++) {gui_str[psigs_i].value = psigs[psigs_i];} gui_str[psigs_i].value = psigs[psigs_i];}

int clamp_int(int val, int min, int max);
float clamp_float(float val, float min, float max);

int min_int(int a, int b);

int max_int(int a, int b);

int string_to_int(char* src, int length);
float string_to_float(char* src, int length);

char digit_to_char(int digit);

int string_length(char* str);