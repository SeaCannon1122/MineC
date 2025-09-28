#include <minec_client.h>

#define PIXELFONT client->gui_state.font

#define PAD(size, pad) (((size) + (pad) - 1) / (pad) * (pad))

const uint32_t fallback_mapping = 0;

const struct bitmap_metadata fallback_metadata = { .width = 8, .thickness = 1 };

const uint8_t fallback_bitmap[8] =
{
	0b11111111,
	0b10000001,
	0b10011001,
	0b10010001,
	0b10011001,
	0b10010001,
	0b10000001,
	0b11111111,
};


void pixelfont_load(struct minec_client* client)
{
	bool valid = true;

	struct asset_loader_asset font_asset;
	asset_loader_get_asset(client, "minec/font.pixelfont", &font_asset);

	struct pixelfont_header* header = font_asset.data;

	if (font_asset.size <= sizeof(struct pixelfont_header)) valid = false;

	if (valid) if (
		font_asset.size < header->header_section_size + header->mappings_section_size + header->bitmap_metadata_section_size + header->bitmap_data_section_size ||
		header->mapping_count == 0 ||
		header->mappings_section_size < header->mapping_count * sizeof(uint32_t) ||
		header->bitmap_count == 0 ||
		header->bitmap_metadata_section_size < header->bitmap_count * sizeof(struct bitmap_metadata) ||
		header->bitmap_data_section_size < header->bitmap_count * header->resolution * header->resolution / 8 ||
		header->resolution < PIXELFONT_MIN_RESOLUTION ||
		header->resolution > PIXELFONT_MAX_RESOLUTION ||
		header->resolution % 8 ||
		header->default_bitmap_index >= header->bitmap_count
	) valid = false;

	uint32_t* mappings = (uintptr_t)header + (uintptr_t)header->header_section_size;
	struct bitmap_metadata* metadata = (uintptr_t)mappings + (uintptr_t)header->mappings_section_size;
	uint32_t* bitmaps = (uintptr_t)metadata + (uintptr_t)header->bitmap_metadata_section_size;

	for (uint32_t i = 0; i < header->mapping_count && valid; i++) if (mappings[i] >= header->bitmap_count) valid = false;

	for (uint32_t i = 0; i < header->bitmap_count && valid; i++) if (metadata[i].width == 0 || metadata[i].width > header->resolution || metadata[i].thickness == 0 || metadata[i].thickness > header->resolution) valid = false;

	if (valid)
	{
		if ((PIXELFONT.data = malloc(
			PAD(header->mapping_count * sizeof(uint32_t), 32) + 
			PAD(header->bitmap_count * sizeof(struct bitmap_metadata), 32) + 
			PAD(header->bitmap_count * header->resolution * header->resolution / 8, 32)
		)) == NULL) valid = false;
		else
		{
			PIXELFONT.mapping_count = header->mapping_count;
			PIXELFONT.bitmap_count = header->bitmap_count;
			PIXELFONT.resolution = header->resolution;

			PIXELFONT.mappings = PIXELFONT.data;
			PIXELFONT.metadata = (uintptr_t)PIXELFONT.mappings + (uintptr_t)PAD(header->mapping_count * sizeof(uint32_t), 32);
			PIXELFONT.bitmap_data = (uintptr_t)PIXELFONT.metadata + (uintptr_t)PAD(header->bitmap_count * sizeof(struct bitmap_metadata), 32);

			memcpy(PIXELFONT.mappings, mappings, header->mapping_count * sizeof(uint32_t));
			memcpy(PIXELFONT.metadata, metadata, header->bitmap_count * sizeof(struct bitmap_metadata));
			memcpy(PIXELFONT.bitmap_data, bitmaps, header->bitmap_count * header->resolution * header->resolution / 8);
		}
	}

	if (valid = false)
	{
		PIXELFONT.data = NULL;
		PIXELFONT.resolution = 8;
		PIXELFONT.mapping_count = 1;
		PIXELFONT.mappings = &fallback_mapping;
		PIXELFONT.bitmap_count = 1;
		PIXELFONT.metadata = &fallback_metadata;
		PIXELFONT.bitmap_data = &fallback_bitmap;

		minec_client_log_info(client, "[GUI] Pixelfont contained invalid data");
	}
	
	if (font_asset.external) free(font_asset.data);
}