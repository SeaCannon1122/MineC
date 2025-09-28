#pragma once

#ifndef STRINGUITLS_H
#define STRINGUITLS_H

#include <stdint.h>
#include <stdarg.h>

uint32_t string8_len(const uint8_t* s);
uint32_t string16_len(const uint16_t* s);
uint32_t string32_len(const uint32_t* s);

int32_t string8_cmp(const uint8_t* s1, const uint8_t* s2);
int32_t string16_cmp(const uint16_t* s1, const uint16_t* s2);
int32_t string32_cmp(const uint32_t* s1, const uint32_t* s2);

uint32_t string8_vsnprintf(uint8_t* buffer, uint32_t buffer_length, const uint8_t* format, va_list args);
uint32_t string16_vsnprintf(uint16_t* buffer, uint32_t buffer_length, const uint16_t* format, va_list args);
uint32_t string32_vsnprintf(uint32_t* buffer, uint32_t buffer_length, const uint32_t* format, va_list args);

uint32_t string8_snprintf(uint8_t* buffer, uint32_t buffer_length, const uint8_t* format, ...);
uint32_t string16_snprintf(uint16_t* buffer, uint32_t buffer_length, const uint16_t* format, ...);
uint32_t string32_snprintf(uint32_t* buffer, uint32_t buffer_length, const uint32_t* format, ...);

uint8_t* string8_malloc(uint8_t* string, ...);
uint16_t* string16_malloc(uint16_t* string, ...);
uint32_t* string32_malloc(uint32_t* string, ...);

#endif