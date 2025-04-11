#include "pixelchar_internal.h"

#define _CHAR_BUFFER_EXTENSION_SIZE 1024

uint32_t pixelchar_renderer_create(struct pixelchar_renderer* pcr, uint32_t max_chars_to_render)
{

	pcr->char_buffer = malloc(sizeof(struct pixelchar) * max_chars_to_render);
	if (pcr->char_buffer == NULL) _DEBUG_CALLBACK_CRITICAL_ERROR_RETURN("pixelchar_renderer_create: malloc failed");

	pcr->char_buffer_length = max_chars_to_render;
	pcr->char_count = 0;
	pcr->backend_selected = PIXELCHAR_RENDERER_BACKEND_NONE;

	return PIXELCHAR_SUCCESS;
}

uint32_t pixelchar_renderer_add_font(struct pixelchar_renderer* pcr, void* font, size_t size, uint32_t index)
{
	struct pixelfont_metadata* metadata = font;

	if (metadata->total_size != size)
		_DEBUG_CALLBACK_ERROR_RETURN("pixelchar_renderer_add_font: size does not match total_size");

	if (metadata->metadata_section_size < sizeof(struct pixelfont_metadata))
		_DEBUG_CALLBACK_ERROR_RETURN("pixelchar_renderer_add_font: metadata_section_size is smaller than minimum size of 'sizeof(struct pixelfont_metadata)'");
	if (metadata->metadata_section_size % 8 != 0)
		_DEBUG_CALLBACK_WARNING("pixelchar_renderer_add_font: metadata_section_size is not a multiple of eight");

	if (metadata->mappings_section_size < metadata->mappings_count * sizeof(uint32_t))
		_DEBUG_CALLBACK_ERROR_RETURN("pixelchar_renderer_add_font: mappings_section_size is smaller than minimum size of 'mapping_table_length * sizeof(uint32_t)'");
	if (metadata->mappings_section_size % 8 != 0)
		_DEBUG_CALLBACK_WARNING("pixelchar_renderer_add_font: mappings_section_size is not a multiple of eight");

	if (metadata->widths_section_size < metadata->bitmaps_count * sizeof(uint8_t))
		_DEBUG_CALLBACK_ERROR_RETURN("pixelchar_renderer_add_font: widths_section_size is smaller than minimum size of 'bitmap_count * sizeof(uint8_t)'");
	if (metadata->widths_section_size % 8 != 0)
		_DEBUG_CALLBACK_WARNING("pixelchar_renderer_add_font: widths_section_size is not a multiple of eight");

	if (metadata->bitmaps_section_size < metadata->bitmaps_count * metadata->resolution * metadata->resolution / 8)
		_DEBUG_CALLBACK_ERROR_RETURN("pixelchar_renderer_add_font: bitmaps_section_size is smaller than minimum size of 'bitmap_count * resolution * resolution / 8'");
	if (metadata->bitmaps_section_size % 8 != 0)
		_DEBUG_CALLBACK_WARNING("pixelchar_renderer_add_font: bitmaps_section_size is not a multiple of eight");

	if (metadata->total_size < metadata->metadata_section_size + metadata->mappings_section_size + metadata->widths_section_size + metadata->bitmaps_section_size)
		_DEBUG_CALLBACK_ERROR_RETURN("pixelchar_renderer_add_font: size does not match sum of section sizes");

	uint32_t* mapping_table = (size_t)font + metadata->metadata_section_size;

	for (uint32_t i = 0; i < metadata->mappings_count; i++)
		if (mapping_table[i] >= metadata->bitmaps_count) _DEBUG_CALLBACK_ERROR_RETURN("pixelchar_renderer_add_font: mapping table index out of bitmaps_count bounds");

	uint32_t* widths = (size_t)font + metadata->metadata_section_size + metadata->mappings_section_size;

	for (uint32_t i = 0; i < metadata->bitmaps_count; i++)
	{
		if (widths[i] > metadata->resolution) _DEBUG_CALLBACK_ERROR_RETURN("pixelchar_renderer_add_font: width found being greater than resolution allows");
	}

	void* memory = malloc(PIXELCHAR_PAD(metadata->mappings_count * sizeof(uint32_t)) + PIXELCHAR_PAD(metadata->bitmaps_count * sizeof(uint8_t)));
	_DEBUG_CALLBACK_CRITICAL_ERROR_RETURN("pixelchar_renderer_add_font: malloc failed");

	uint32_t ret;// = pixelchar_renderer_backend_add_font_functions[pcr->backend_selected](pcr, font, index);;
	if (ret == PIXELCHAR_FAILED) return PIXELCHAR_FAILED;

	uint32_t* mappings_ = memory;
	uint8_t* widths_ = (size_t)memory + PIXELCHAR_PAD(metadata->mappings_count * sizeof(uint32_t));

	return PIXELCHAR_SUCCESS;
}

void pixelchar_renderer_queue_pixelchars(struct pixelchar_renderer* pcr, struct pixelchar* chars, uint32_t char_count)
{
	uint32_t chars_to_copy_count = 0;

	if (pcr->char_buffer_length - pcr->char_count < char_count)
	{
		_DEBUG_CALLBACK_ERROR("pixelchar_renderer_queue_pixelchars: total queued char count exceeds maximum");
		chars_to_copy_count = pcr->char_buffer_length - pcr->char_count;
	}
	else chars_to_copy_count = char_count;

	if (chars_to_copy_count > 0)
	{
		memcpy(&pcr->char_buffer[pcr->char_count], chars, sizeof(struct pixelchar) * chars_to_copy_count);
		pcr->char_count += chars_to_copy_count;
	}
}

void pixelchar_renderer_backend_deinit(struct pixelchar_renderer* pcr)
{
	if (pcr->backend_selected == PIXELCHAR_RENDERER_BACKEND_NONE) _DEBUG_CALLBACK_ERROR("pixelchar_renderer_backend_deinit: no backend initialized");
	else
	{
		pixelchar_renderer_backend_deinit_functions[pcr->backend_selected](pcr);
		pcr->backend_selected = PIXELCHAR_RENDERER_BACKEND_NONE;
	}		
}

void pixelchar_renderer_destroy(struct pixelchar_renderer* pcr)
{
	if (pcr->backend_selected != PIXELCHAR_RENDERER_BACKEND_NONE)
	{
		_DEBUG_CALLBACK_WARNING("pixelchar_renderer_destroy: backend should be deinitialized by the user before destroying pixelchar_renderer");
		pixelchar_renderer_backend_deinit(pcr);
	}

	free(pcr->char_buffer);
}