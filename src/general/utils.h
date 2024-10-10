#pragma once

#include <stdio.h>
#include <time.h>

#define parse_string_into_gui_string(src, gui_str) { char* psigs = src; int psigs_i = 0; for(; psigs[psigs_i] != '\0'; psigs_i++) {gui_str[psigs_i].value = psigs[psigs_i];} gui_str[psigs_i].value = psigs[psigs_i];}

#define parse_string(src, dest) {char* _src_str = src; int _str_i = 0; for(; _src_str[_str_i] != '\0'; _str_i++) {dest[_str_i] = _src_str[_str_i];} dest[_str_i] = _src_str[_str_i];}

#define IF_BIT(ptr, pos) (((char*)ptr)[pos / 8] & (1 << (pos % 8)) )
#define SET_BIT_1(ptr, pos) (((char*)ptr)[pos / 8] |= (1 << (pos % 8) ))
#define SET_BIT_0(ptr, pos) (((char*)ptr)[pos / 8] &= (0xFF ^ (1 << (pos % 8) )))

int clamp_int(int val, int min, int max);
float clamp_float(float val, float min, float max);

int min_int(int a, int b);

int max_int(int a, int b);

int string_to_int(char* src, int length);
float string_to_float(char* src, int length);

char digit_to_char(int digit);

int string_length(char* str);

void get_time_in_string(char* buffer, time_t raw_time);