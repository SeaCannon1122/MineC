#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "general/pixelchar/src/pixelchar.h"


int main(int argc, char* argv[]) {

	uint32_t mapping_table[256];
	for (int i = 0; i < 256; i++) mapping_table[i] = 0;
	mapping_table['A'] = 1;

	uint32_t bitmaps[2][2];
	bitmaps[0][0] = 0b01010101101010100101010110101010;
	bitmaps[0][1] = 0b01010101101010100101010110101010;

	bitmaps[1][0] = 0xffff0000;
	bitmaps[1][1] = 0xf0f0f0f0;

	uint8_t widths[2] = { 8, 8 };

	struct pixelchar_font_metadata metadata;
	metadata.resolution = PIXELCHAR_FONT_RESOLUTION_8X8;
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