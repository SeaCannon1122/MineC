#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <pixelchar/pixelchar.h>

struct pixel_font {
	struct {
		uint64_t width;
		int layout[8];
	} char_font_entries[0x20000];
};

struct pixel_font* load_pixel_font(char* src) {

	FILE* file = fopen(src, "rb");
	if (file == NULL) return NULL;

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	rewind(file);

	void* buffer = calloc(1, sizeof(struct pixel_font));
	if (buffer == NULL) {
		fclose(file);
		return NULL;
	}

	size_t bytesRead = fread(buffer, 1, fileSize, file);

	fclose(file);

	return buffer;
}

int main(int argc, char* argv[]) {

	struct pixel_font* font = load_pixel_font("../../../resources/client/assets/fonts/default.pixelfont");

	uint32_t mapping_table[130];
	uint16_t bitmaps[130][16];
	uint8_t widths[130];

	for (int i = 0; i < 130; i++) {
		mapping_table[i] = i;
		widths[i] = font->char_font_entries[i].width;
		memcpy(&bitmaps[i][0], &font->char_font_entries[i].layout[0], 16 * 16 / 8);
	}

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

	FILE* file = fopen("default.pixelchar_font", "wb");

	size_t written = fwrite(font_data, 1, metadata.total_size, file);
	if (written != metadata.total_size) {
		perror("Error writing data\n");
		printf("size %lld written %lld\n", metadata.total_size, written);
	}

	fclose(file);

	return 0;
}