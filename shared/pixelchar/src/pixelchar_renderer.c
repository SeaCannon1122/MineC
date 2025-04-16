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
		}
		pcr->fonts[index]->references--;
	}

	pcr->fonts[index] = font;

	if (pcr->fonts[index] != NULL) {
		for (uint32_t i = 0; (1 << i) < PIXELCHAR_BACKEND_MAX_BIT; i++) if (pcr->backends_initialized & (1 << i))
		{
			_pixelchar_font_backend_reference_add_functions[i](pcr->fonts[index], pcr, index);
		}
		pcr->fonts[index]->references++;
	}
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

	for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONTS; i++)
	{
		if (pcr->fonts[i] != NULL) pcr->fonts[i]->references--;
	}

	free(pcr->char_buffer);
}

void _pixelchar_renderer_render_convert_to_internal_characters(struct pixelchar_renderer* pcr)
{
	struct internal_pixelchar* chars = pcr->char_buffer;

	for (uint32_t i = 0; i < pcr->char_count; i++)
	{
		uint32_t value = pcr->char_buffer[i].value;
		if (chars[i].font >= PIXELCHAR_RENDERER_MAX_FONTS)
		{
			chars[i].font = PIXELFONT_INVALID;
			chars[i].bitmap_width = 8;
			chars[i].font_resolution = 8;
		}
		else if (pcr->fonts[chars[i].font] == NULL)
		{
			chars[i].font = PIXELFONT_INVALID;
			chars[i].bitmap_width = 8;
			chars[i].font_resolution = 8;
		}
		else
		{
			if (value >= pcr->fonts[chars[i].font]->mappings_count)
				chars[i].bitmap_index = 0;
			else
				chars[i].bitmap_index = pcr->fonts[chars[i].font]->mappings[value];

			chars[i].bitmap_width = pcr->fonts[chars[i].font]->widths[chars[i].bitmap_index];
			chars[i].font_resolution = pcr->fonts[chars[i].font]->resolution;
		}
	}
}

int16_t pixelchar_renderer_get_pixelchar_width(struct pixelchar_renderer* pcr, struct pixelchar* c)
{
	if (c->font >= PIXELCHAR_RENDERER_MAX_FONTS) return (int32_t)c->scale * 8;
	if (pcr->fonts[c->font] == NULL) return (int32_t)c->scale * 8;

	uint32_t bitmap_index;
	if (c->value >= pcr->fonts[c->font]->mappings_count) bitmap_index = 0;
	else bitmap_index = pcr->fonts[c->font]->mappings[c->value];

	return ((int32_t)pcr->fonts[c->font]->widths[bitmap_index] * (int32_t)c->scale * 8 + (int32_t)pcr->fonts[c->font]->resolution - 1) / (int32_t)pcr->fonts[c->font]->resolution;
}

int16_t pixelchar_renderer_get_pixelchar_spacing(struct pixelchar_renderer* pcr, struct pixelchar* c0, struct pixelchar* c1)
{
	return ((int32_t)c0->scale * 8 + 15) / 16 + ((int32_t)c1->scale * 8) / 16;
}