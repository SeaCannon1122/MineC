#include "pixelchar_internal.h"

uint32_t pixelchar_font_create(struct pixelchar_font* font, void* font_file_data, size_t font_file_data_size)
{
	struct pixelchar_font_metadata* metadata = font_file_data;

	size_t bitmap_size = metadata->resolution * metadata->resolution / 8;

	if (metadata->total_size != font_file_data_size)
		_DEBUG_CALLBACK_ERROR_RETURN("pixelchar_font_create: size does not match total_size");

	if (metadata->metadata_section_size < sizeof(struct pixelchar_font_metadata))
		_DEBUG_CALLBACK_ERROR_RETURN("pixelchar_font_create: metadata_section_size is smaller than minimum size of 'sizeof(struct pixelfont_metadata)'");
	if (metadata->metadata_section_size % 8 != 0)
		_DEBUG_CALLBACK_WARNING("pixelchar_font_create: metadata_section_size is not a multiple of eight");

	if (metadata->mappings_section_size < metadata->mappings_count * sizeof(uint32_t))
		_DEBUG_CALLBACK_ERROR_RETURN("pixelchar_font_create: mappings_section_size is smaller than minimum size of 'mapping_table_length * sizeof(uint32_t)'");
	if (metadata->mappings_section_size % 8 != 0)
		_DEBUG_CALLBACK_WARNING("pixelchar_font_create: mappings_section_size is not a multiple of eight");

	if (metadata->widths_section_size < metadata->bitmaps_count * sizeof(uint8_t))
		_DEBUG_CALLBACK_ERROR_RETURN("pixelchar_font_create: widths_section_size is smaller than minimum size of 'bitmap_count * sizeof(uint8_t)'");
	if (metadata->widths_section_size % 8 != 0)
		_DEBUG_CALLBACK_WARNING("pixelchar_font_create: widths_section_size is not a multiple of eight");

	if (metadata->bitmaps_section_size < bitmap_size * metadata->bitmaps_count)
		_DEBUG_CALLBACK_ERROR_RETURN("pixelchar_font_create: bitmaps_section_size is smaller than minimum size of 'bitmap_count * resolution * resolution / 8'");
	if (metadata->bitmaps_section_size % 8 != 0)
		_DEBUG_CALLBACK_WARNING("pixelchar_font_create: bitmaps_section_size is not a multiple of eight");

	if (metadata->total_size < metadata->metadata_section_size + metadata->mappings_section_size + metadata->widths_section_size + metadata->bitmaps_section_size)
		_DEBUG_CALLBACK_ERROR_RETURN("pixelchar_font_create: size does not match sum of section sizes");

	if (metadata->resolution != 8 && metadata->resolution != 16 && metadata->resolution != 32 && metadata->resolution != 64 && metadata->resolution != 128)
		_DEBUG_CALLBACK_ERROR_RETURN("pixelchar_font_create: resolution is not of supported size");

	uint32_t* mapping_table = (size_t)font_file_data + metadata->metadata_section_size;

	for (uint32_t i = 0; i < metadata->mappings_count; i++)
		if (mapping_table[i] >= metadata->bitmaps_count) _DEBUG_CALLBACK_ERROR_RETURN("pixelchar_font_create: mapping table index out of bitmaps_count bounds");

	uint8_t* widths = (size_t)font_file_data + metadata->metadata_section_size + metadata->mappings_section_size;

	for (uint32_t i = 0; i < metadata->bitmaps_count; i++)
	{
		if (widths[i] > metadata->resolution) _DEBUG_CALLBACK_ERROR_RETURN("pixelchar_font_create: width found to be greater than resolution allows");
	}

	void* bitmaps = (size_t)font_file_data + metadata->metadata_section_size + metadata->mappings_section_size + metadata->widths_section_size;

	font->memory_handle = malloc(PIXELCHAR_PAD(metadata->mappings_count * sizeof(uint32_t)) + PIXELCHAR_PAD(metadata->bitmaps_count * sizeof(uint8_t)) + PIXELCHAR_PAD(metadata->bitmaps_count * bitmap_size));
	if (font->memory_handle == NULL)
		_DEBUG_CALLBACK_CRITICAL_ERROR_RETURN("pixelchar_font_create: malloc failed");

	font->mappings_count = metadata->mappings_count;
	font->mappings = font->memory_handle;
	memcpy(font->mappings, mapping_table, sizeof(uint32_t) * metadata->mappings_count);

	font->bitmaps_count = metadata->bitmaps_count;
	font->widths = (size_t)font->memory_handle + PIXELCHAR_PAD(metadata->mappings_count * sizeof(uint32_t));
	memcpy(font->widths, widths, sizeof(uint8_t) * metadata->bitmaps_count);
	font->bitmaps = (size_t)font->memory_handle + PIXELCHAR_PAD(metadata->mappings_count * sizeof(uint32_t)) + PIXELCHAR_PAD(metadata->bitmaps_count * sizeof(uint8_t));
	memcpy(font->bitmaps, bitmaps, bitmap_size * metadata->bitmaps_count);

	font->resolution = metadata->resolution;
	font->references = 0;

	for (uint32_t i = 0; (1 << i) < PIXELCHAR_BACKEND_MAX_BIT; i++)
		if (_pixelchar_font_backend_reference_init_functions[i] != NULL)
			_pixelchar_font_backend_reference_init_functions[i](font);

	return PIXELCHAR_SUCCESS;
}

uint32_t pixelchar_font_destroy(struct pixelchar_font* font)
{
	if (font->references != 0) _DEBUG_CALLBACK_ERROR_RETURN("pixelchar_font_destroy: font is still referenced by some pixelchar_renderer(s)");

	free(font->memory_handle);

	return PIXELCHAR_SUCCESS;
}
