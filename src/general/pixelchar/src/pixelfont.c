#include "pixelfont.h"

#include <stdio.h>
#include <malloc.h>
#include <string.h>

struct pixelfont* pixelchar_load_font(uint8_t* src) {

	FILE* file = fopen(src, "rb");
	if (file == NULL) return NULL;

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	rewind(file);

	void* buffer = calloc(1, sizeof(struct pixelfont));
	if (buffer == NULL) {
		fclose(file);
		return NULL;
	}

	size_t bytesRead = fread(buffer, 1, fileSize, file);

	fclose(file);

	return buffer;
}