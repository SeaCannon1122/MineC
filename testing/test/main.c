#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <pixelchar/pixelchar.h>

void* loadFile(uint8_t* src, size_t* size) {

	FILE* file = fopen(src, "rb");
	if (file == NULL) return NULL;

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	rewind(file);

	void* buffer = calloc(1, fileSize);
	if (buffer == NULL) {
		fclose(file);
		return NULL;
	}

	*size = fread(buffer, 1, fileSize, file);

	fclose(file);

	return buffer;
}

int main(int argc, char* argv[]) {

	size_t bitmap_data_size;
	void* bitmap_data = loadFile("../../../scripts/pixelfont_creator/bigfont.bin", &bitmap_data_size);

	uint32_t mapping_table[1] = {0};
	uint64_t bitmaps[1][2 * 128];
	uint8_t widths[1] = {128};

	memcpy(bitmaps, bitmap_data, sizeof(bitmaps[0]));

	struct pixelchar_font_metadata metadata;
	metadata.resolution = PIXELCHAR_FONT_RESOLUTION_128X128;
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

	FILE* file = fopen("giant.pixelchar_font", "wb");

	size_t written = fwrite(font_data, 1, metadata.total_size, file);
	if (written != metadata.total_size) {
		perror("Error writing data\n");
		printf("size %lld written %lld\n", metadata.total_size, written);
	}

	fclose(file);

	return 0;
}