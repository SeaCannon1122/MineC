#include "../include/pixelchar/pixelchar.h"

PIXELCHAR_DEBUG_CALLBACK_FUNCTION debug_callback_function = NULL;

uint32_t pixelchar_set_debug_callback(PIXELCHAR_DEBUG_CALLBACK_FUNCTION callback_function)
{
	debug_callback_function = callback_function;
}