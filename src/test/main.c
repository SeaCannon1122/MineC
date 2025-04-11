#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "general/pixelchar/src/pixelchar.h"


int main(int argc, char* argv[]) {
	
	uint32_t mapping_table[256];
	for (int i = 0; i < 256; i++) mapping_table[i] = 0;
	mapping_table['A'] = 1;

	struct pixelfont_bitmap_8x8 bitmaps[2];
	bitmaps[0] = (struct pixelfont_bitmap_8x8){0b01010101101010100101010110101010, 0b01010101101010100101010110101010};
	bitmaps[1] = (struct pixelfont_bitmap_8x8){0xffff0000, 0xf0f0f0f0};

	uint8_t widths[2] = { 8, 8 };

	struct pixelfont_metadata metadata;
	metadata.resolution = PIXELFONT_RESOLUTION_16X16;
	metadata.mapping_table_length = sizeof(mapping_table) / sizeof(mapping_table[0]);
	metadata.bitmap_count = sizeof(bitmaps) / sizeof(bitmaps[0]);
	metadata.total_size = sizeof(metadata) + sizeof(mapping_table) + sizeof(widths) + sizeof(bitmaps);
	metadata.name[0] = "t";
	metadata.name[1] = "e";
	metadata.name[2] = "s";
	metadata.name[3] = "t";
	metadata.name[4] = "\0";

	size_t size = sizeof(metadata) + sizeof(mapping_table) + sizeof(bitmaps);
	void* font_data = malloc(size);

	memcpy(font_data, &metadata, sizeof(metadata));
	memcpy((size_t)font_data + sizeof(metadata), &mapping_table, sizeof(mapping_table));
	memcpy((size_t)font_data + sizeof(metadata) + sizeof(mapping_table), &bitmaps, sizeof(bitmaps));

	FILE* file = fopen("test.pixelfont", "wb");

	size_t written = fwrite(font_data, 1, size, file);
	if (written != size) {
		perror("Error writing data\n");
		printf("size %lld written %lld\n", size, written);
	}

	fclose(file);

	return 0;
}