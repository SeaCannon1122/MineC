#include <stringutils.h>

#include <stdarg.h>
#include <utils.h>
#include <stdlib.h>

uint32_t string8_len(const uint8_t* s)
{
	for (uint32_t length = 0; ; length++) if (s[length] == 0) return length;
}

uint32_t string16_len(const uint16_t* s)
{
	for (uint32_t length = 0; ; length++) if (s[length] == 0) return length;
}

uint32_t string32_len(const uint32_t* s)
{
	for (uint32_t length = 0; ; length++) if (s[length] == 0) return length;
}

int32_t string8_cmp(const uint8_t* RESTRICT s1, const uint8_t* RESTRICT s2)
{
	for (; ; s1++, s2++) if (*s1 == 0 || *s1 != *s2) return (int32_t)(*s1 - *s2);
}

int32_t string16_cmp(const uint16_t* RESTRICT s1, const uint16_t* RESTRICT s2)
{
	for (; ; s1++, s2++) if (*s1 == 0 || *s1 != *s2) return (int32_t)(*s1 - *s2);
}

int32_t string32_cmp(const uint32_t* RESTRICT s1, const uint32_t* RESTRICT s2)
{
	for (; ; s1++, s2++) if (*s1 == 0 || *s1 != *s2) return (int32_t)(*s1 - *s2);
}


uint32_t string8_snprintf(uint8_t* RESTRICT buffer, uint32_t buffer_length, const uint8_t* RESTRICT format, ...)
{
    va_list args;
    va_start(args, format);
    uint32_t n = string8_vsnprintf(buffer, buffer_length, format, args);
    va_end(args);
    return n;
}

uint32_t string8_vsnprintf(uint8_t* RESTRICT buffer, uint32_t buffer_length, const uint8_t* RESTRICT format, va_list args)
{
    uint8_t* buf = buffer;
    size_t rem = buffer_length;

    for (; *format; format++)
    {
        if (*format != '%')
        {
            if (rem > 1) { *buf = *format; rem--; }
            buf++;
            continue;
        }

        format++;
        uint32_t precision = 6; 

        if (*format == '.')
        {
            format++;
            precision = 0;
            for (; *format >= '0' && *format <= '9'; format++) precision = precision * 10 + (*format - '0');
        }

        switch (*format)
        {
        case 'd':
        {
            int32_t val = va_arg(args, int32_t);
            uint8_t tmp[12];
            uint8_t* t = tmp + sizeof(tmp) - 1;
            *t = '\0';
            int32_t neg = val < 0;
            uint32_t u = neg ? -val : val;
            do { *--t = '0' + u % 10; u /= 10; } while (u);
            if (neg) *--t = '-';
            while (*t) { if (rem > 1) { *buf = *t; rem--; } buf++;  t++; }
        } break;
        case 'u':
        {
            uint32_t u = va_arg(args, uint32_t);
            uint8_t tmp[11];
            uint8_t* t = tmp + sizeof(tmp) - 1;
            *t = '\0';
            do { *--t = '0' + u % 10; u /= 10; } while (u);
            while (*t) { if (rem > 1) { *buf = *t; rem--; } buf++;  t++; }
        } break;
        case 'f':
        {
            double d = va_arg(args, double);
            if (d < 0) { if (rem > 1) { *buf = '-'; rem--; } d = -d; buf++; }
            int32_t whole = (int32_t)d;
            double frac = d - whole;

            uint8_t tmp[12];
            uint8_t* t = tmp + sizeof(tmp) - 1;
            *t = '\0';
            uint32_t u = whole;
            do { *--t = '0' + u % 10; u /= 10; } while (u);
            while (*t) { if (rem > 1) { *buf = *t; rem--; } buf++;  t++;}

            if (rem > 1) { *buf = '.'; rem--; }
            buf++;

            for (uint32_t i = 0; i < precision; i++)
            {
                frac *= 10;
                uint32_t digit = (uint32_t)frac;
                if (rem > 1) { *buf = '0' + digit; rem--; }
                buf++;
                frac -= digit;
            }
        } break;
        case 's':
        {
            const uint8_t* s = va_arg(args, const uint8_t*);
            while (*s) { if (rem > 1) { *buf = *s; rem--; } buf++;  s++; }
        } break;
        case '%':
        {
            if (rem > 1) { *buf = '%'; rem--; buf++; }
            
        } break;
        }
    }

    if (rem > 0) *buf = '\0';
    return (uint32_t)(buf - buffer);
}

uint32_t string16_snprintf(uint16_t* RESTRICT buffer, uint32_t buffer_length, const uint16_t* RESTRICT format, ...)
{
    va_list args;
    va_start(args, format);
    uint32_t n = string16_vsnprintf(buffer, buffer_length, format, args);
    va_end(args);
    return n;
}

uint32_t string16_vsnprintf(uint16_t* RESTRICT buffer, uint32_t buffer_length, const uint16_t* RESTRICT format, va_list args)
{
    uint16_t* buf = buffer;
    size_t rem = buffer_length;

    for (; *format; format++)
    {
        if (*format != '%')
        {
            if (rem > 1) { *buf++ = *format; rem--; }
            continue;
        }

        format++;
        uint32_t precision = 6;

        if (*format == '.')
        {
            format++;
            precision = 0;
            for (; *format >= '0' && *format <= '9'; format++) precision = precision * 10 + (*format - '0');
        }

        switch (*format)
        {
        case 'd':
        {
            int32_t val = va_arg(args, int32_t);
            uint16_t tmp[12];
            uint16_t* t = tmp + sizeof(tmp) - 1;
            *t = '\0';
            int32_t neg = val < 0;
            uint32_t u = neg ? -val : val;
            do { *--t = '0' + u % 10; u /= 10; } while (u);
            if (neg) *--t = '-';
            while (*t) if (rem > 1) { *buf++ = *t++; rem--; }
            else t++;
        } break;
        case 'u':
        {
            uint32_t u = va_arg(args, uint32_t);
            uint16_t tmp[11];
            uint16_t* t = tmp + sizeof(tmp) - 1;
            *t = '\0';
            do { *--t = '0' + u % 10; u /= 10; } while (u);
            while (*t) if (rem > 1) { *buf++ = *t++; rem--; }
            else t++;
        } break;
        case 'f':
        {
            double d = va_arg(args, double);
            if (d < 0) { if (rem > 1) { *buf++ = '-'; rem--; } d = -d; }
            int32_t whole = (int32_t)d;
            double frac = d - whole;

            uint16_t tmp[12];
            uint16_t* t = tmp + sizeof(tmp) - 1;
            *t = '\0';
            uint32_t u = whole;
            do { *--t = '0' + u % 10; u /= 10; } while (u);
            while (*t) if (rem > 1) { *buf++ = *t++; rem--; }
            else t++;

            if (rem > 1) { *buf++ = '.'; rem--; }

            for (uint32_t i = 0; i < precision; i++)
            {
                frac *= 10;
                uint32_t digit = (uint32_t)frac;
                if (rem > 1) { *buf++ = '0' + digit; rem--; }
                frac -= digit;
            }
        } break;
        case 's':
        {
            const uint16_t* s = va_arg(args, const uint16_t*);
            while (*s) if (rem > 1) { *buf++ = *s++; rem--; }
            else s++;
        } break;
        case '%':
        {
            if (rem > 1) { *buf++ = '%'; rem--; }

        } break;
        }
    }

    if (rem > 0) *buf = '\0';
    return (uint32_t)(buf - buffer);
}

uint32_t string32_snprintf(uint32_t* RESTRICT buffer, uint32_t buffer_length, const uint32_t* RESTRICT format, ...)
{
    va_list args;
    va_start(args, format);
    uint32_t n = string32_vsnprintf(buffer, buffer_length, format, args);
    va_end(args);
    return n;
}

uint32_t string32_vsnprintf(uint32_t* RESTRICT buffer, uint32_t buffer_length, const uint32_t* RESTRICT format, va_list args)
{
    uint32_t* buf = buffer;
    size_t rem = buffer_length;

    for (; *format; format++)
    {
        if (*format != '%')
        {
            if (rem > 1) { *buf++ = *format; rem--; }
            continue;
        }

        format++;
        uint32_t precision = 6;

        if (*format == '.')
        {
            format++;
            precision = 0;
            for (; *format >= '0' && *format <= '9'; format++) precision = precision * 10 + (*format - '0');
        }

        switch (*format)
        {
        case 'd':
        {
            int32_t val = va_arg(args, int32_t);
            uint32_t tmp[12];
            uint32_t* t = tmp + sizeof(tmp) - 1;
            *t = '\0';
            int32_t neg = val < 0;
            uint32_t u = neg ? -val : val;
            do { *--t = '0' + u % 10; u /= 10; } while (u);
            if (neg) *--t = '-';
            while (*t) if (rem > 1) { *buf++ = *t++; rem--; }
            else t++;
        } break;
        case 'u':
        {
            uint32_t u = va_arg(args, uint32_t);
            uint32_t tmp[11];
            uint32_t* t = tmp + sizeof(tmp) - 1;
            *t = '\0';
            do { *--t = '0' + u % 10; u /= 10; } while (u);
            while (*t) if (rem > 1) { *buf++ = *t++; rem--; }
            else t++;
        } break;
        case 'f':
        {
            double d = va_arg(args, double);
            if (d < 0) { if (rem > 1) { *buf++ = '-'; rem--; } d = -d; }
            int32_t whole = (int32_t)d;
            double frac = d - whole;

            uint32_t tmp[12];
            uint32_t* t = tmp + sizeof(tmp) - 1;
            *t = '\0';
            uint32_t u = whole;
            do { *--t = '0' + u % 10; u /= 10; } while (u);
            while (*t) if (rem > 1) { *buf++ = *t++; rem--; }
            else t++;

            if (rem > 1) { *buf++ = '.'; rem--; }

            for (uint32_t i = 0; i < precision; i++)
            {
                frac *= 10;
                uint32_t digit = (uint32_t)frac;
                if (rem > 1) { *buf++ = '0' + digit; rem--; }
                frac -= digit;
            }
        } break;
        case 's':
        {
            const uint32_t* s = va_arg(args, const uint32_t*);
            while (*s) if (rem > 1) { *buf++ = *s++; rem--; }
            else s++;
        } break;
        case '%':
        {
            if (rem > 1) { *buf++ = '%'; rem--; }

        } break;
        }
    }

    if (rem > 0) *buf = '\0';
    return (uint32_t)(buf - buffer);
}

uint8_t* string8_malloc(uint8_t* string, ...)
{
    va_list args;
    va_start(args, string);
    uint32_t buffer_length = string8_vsnprintf(NULL, 0, string, args) + 1;
    va_end(args);

    uint8_t* buffer = malloc(buffer_length * sizeof(uint8_t));
    if (buffer == NULL) return NULL;

    va_start(args, string);
    string8_vsnprintf(buffer, buffer_length, string, args);
    va_end(args);

    return buffer;
}

uint16_t* string16_malloc(uint16_t* string, ...)
{
    va_list args;
    va_start(args, string);
    uint32_t buffer_length = string8_vsnprintf(NULL, 0, string, args) + 1;
    va_end(args);

    uint16_t* buffer = malloc(buffer_length * sizeof(uint16_t));
    if (buffer == NULL) return NULL;

    va_start(args, string);
    string8_vsnprintf(buffer, buffer_length, string, args);
    va_end(args);

    return buffer;
}

uint32_t* string32_malloc(uint32_t* string, ...)
{
    va_list args;
    va_start(args, string);
    uint32_t buffer_length = string8_vsnprintf(NULL, 0, string, args) + 1;
    va_end(args);

    uint32_t* buffer = malloc(buffer_length * sizeof(uint32_t));
    if (buffer == NULL) return NULL;

    va_start(args, string);
    string8_vsnprintf(buffer, buffer_length, string, args);
    va_end(args);

    return buffer;
}