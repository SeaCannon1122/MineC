#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <pixelchar/pixelchar.h>


int main(int argc, char* argv[]) {

	uint32_t mapping_table[256];
	for (int i = 0; i < 256; i++) mapping_table[i] = 0;
	mapping_table['A'] = 1;

	uint16_t bitmaps[2][16] = {
		{
			0b0000000000111111,
			0b0000000000111111,

			0b0000000000001100,
			0b0000000000001100,

			0b0000000000001100,
			0b0000000000001100,

			0b0000000000001100,
			0b0000000000001100,

			0b0000000000001100,
			0b0000000000001100,

			0b0000000000001100,
			0b0000000000001100,

			0b0000000000111111,
			0b0000000000111111,

			0b0000000000000000,
			0b0000000000000000,
		},
		{
			0b0000000011111100,
			0b0000000011111100,

			0b0000001100000011,
			0b0000001100000011,

			0b0000001111111111,
			0b0000001111111111,

			0b0000001100000011,
			0b0000001100000011,

			0b0000001100000011,
			0b0000001100000011,

			0b0000001100000011,
			0b0000001100000011,

			0b0000001100000011,
			0b0000001100000011,

			0b0000000000000000,
			0b0000000000000000,
		},
	};

	uint8_t widths[2] = { 6, 10 };

	struct pixelchar_font_metadata metadata;
	metadata.resolution = PIXELCHAR_FONT_RESOLUTION_16X16;
	metadata.mappings_count = sizeof(mapping_table) / sizeof(mapping_table[0]);
	metadata.bitmaps_count = sizeof(bitmaps) / sizeof(bitmaps[0]);
	metadata.total_size = sizeof(metadata) + sizeof(mapping_table) + sizeof(widths) + sizeof(bitmaps);
	metadata.metadata_section_size = sizeof(metadata);
	metadata.mappings_section_size = sizeof(mapping_table);
	metadata.widths_section_size = sizeof(widths);
	metadata.bitmaps_section_size = sizeof(bitmaps);
	metadata.name[0] = "t";
	metadata.name[1] = "e";
	metadata.name[2] = "s";
	metadata.name[3] = "t";
	metadata.name[4] = "\0";

	void* font_data = malloc(metadata.total_size);

	memcpy(font_data, &metadata, sizeof(metadata));
	memcpy((size_t)font_data + sizeof(metadata), &mapping_table, sizeof(mapping_table));
	memcpy((size_t)font_data + sizeof(metadata) + sizeof(mapping_table), &widths, sizeof(widths));
	memcpy((size_t)font_data + sizeof(metadata) + sizeof(mapping_table) + sizeof(widths), &bitmaps, sizeof(bitmaps));

	FILE* file = fopen("test.pixelfont", "wb");

	size_t written = fwrite(font_data, 1, metadata.total_size, file);
	if (written != metadata.total_size) {
		perror("Error writing data\n");
		printf("size %lld written %lld\n", metadata.total_size, written);
	}

	fclose(file);

	return 0;
}