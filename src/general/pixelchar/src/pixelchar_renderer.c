#include "pixelchar_internal.h"

#define INVALID_FONT 255


uint32_t pixelchar_renderer_create(struct pixelchar_renderer* pcr, uint32_t max_chars_to_render)
{

	pcr->char_buffer = malloc(sizeof(struct pixelchar) * max_chars_to_render);
	if (pcr->char_buffer == NULL) _DEBUG_CALLBACK_CRITICAL_ERROR_RETURN("pixelchar_renderer_create: malloc failed");

	pcr->char_buffer_length = max_chars_to_render;
	pcr->char_count = 0;
	pcr->backends_initialized = 0;

	for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONTS; i++) pcr->fonts[i] = NULL;

	return PIXELCHAR_SUCCESS;
}

void pixelchar_renderer_set_font(struct pixelchar_renderer* pcr, struct pixelchar_font* font, uint32_t index)
{
	if (index >= PIXELCHAR_RENDERER_MAX_FONTS) _DEBUG_CALLBACK_ERROR_RETURN("pixelchar_renderer_set_font: index greater or equal than PIXELCHAR_RENDERER_MAX_FONTS");

	if (pcr->fonts[index] != NULL)
	{
		for (uint32_t i = 0; (1 << i) < PIXELCHAR_BACKEND_MAX_BIT; i++) if (pcr->backends_initialized & (1 << i))
		{
			_pixelchar_font_backend_reference_subtract_functions[i](pcr->fonts[index]);
			_pixelchar_font_backend_reference_add_functions[i](font, pcr, index);
		}

		pcr->fonts[index]->references--;
		font->references++;
	}

	pcr->fonts[index] = font;
}

void pixelchar_renderer_queue_pixelchars(struct pixelchar_renderer* pcr, struct pixelchar* chars, uint32_t char_count)
{
	uint32_t chars_to_copy_count = 0;

	if (pcr->char_buffer_length - pcr->char_count < char_count)
	{
		_DEBUG_CALLBACK_WARNING("pixelchar_renderer_queue_pixelchars: total queued char count exceeds maximum");
		chars_to_copy_count = pcr->char_buffer_length - pcr->char_count;
	}
	else chars_to_copy_count = char_count;

	if (chars_to_copy_count > 0)
	{
		memcpy(&pcr->char_buffer[pcr->char_count], chars, sizeof(struct pixelchar) * chars_to_copy_count);

		pcr->char_count += chars_to_copy_count;
	}
}

void pixelchar_renderer_destroy(struct pixelchar_renderer* pcr)
{
	if (pcr->backends_initialized != 0)
	{
		_DEBUG_CALLBACK_WARNING("pixelchar_renderer_destroy: some backends are still initialized. deinitializing ...");

		for (uint32_t i = 0; (1 << i) < PIXELCHAR_BACKEND_MAX_BIT; i++) if (pcr->backends_initialized & (1 << i))
			pixelchar_renderer_backend_deinit_functions[i](pcr);
	}

	free(pcr->char_buffer);
}