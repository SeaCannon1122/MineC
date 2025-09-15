# Pixelfont file format

File extension: .pixelfont

# Structure

the file consists of 4 sections, which are right next to each other

[HEADER SECTION][MAPPINGS SECTION][BITMAP METADATA SECTION][BITMAP DATA SECTION]

All sections should be padded to a multiple of 32 bytes

## Header section

Contains just this header


´´´
#define PIXELFONT_MAX_BUFFER_SIZE 32

#define PIXELFONT_MIN_RESOLUTION 8
#define PIXELFONT_MAX_RESOLUTION 4096

struct pixelfont_header
{
	uint32_t header_section_size;
	uint32_t mappings_section_size;
	uint32_t bitmap_metadata_section_size;
	uint32_t bitmap_data_section_size;

	uint8_t name[PIXELFONT_MAX_BUFFER_SIZE];
	uint32_t mapping_count;
	uint32_t default_bitmap_index;
	uint32_t resolution;
	uint32_t bitmap_count;
};
´´´

header_section_size: size of the header section in bytes
mappings_section_size: size of the mappings section in bytes
bitmap_metadata_section_size: size of the bitmap metadata section in bytes
bitmap_data_section_size: size of the bitmap data section in bytes

name: buffer for NULL terminated ascii string that constains the name of the font

mapping_count: the first mapping_count unicode code points [0, mapping_count - 1] are mapped to a bitmap via the mappings section
default_bitmap_index: unicode code points over that are mapped to the bitmap at this index

resolution: bitmaps are of the size resolution x resolution (resolution x resolution / 8 bytes); resolution must be a multiple of 8; PIXELFONT_MIN_RESOLUTION <= resolution <= PIXELFONT_MAX_RESOLUTION

bitmap_count: the amount of bitmaps contained in the bitmap data secion, each bitmap also has metadata in the bitmap metadata section


## Mappings section

Constains an uint32_t array of length pixelfont_header.mapping_count, which map the unicode coce points to a bitmap via a bitmap index

## Bitmap metadata section

Contains a

struct bitmap_metadata
{
	uint32_t width;
	uint32_t thickness;
};

width: width of the character stored in the bitmap, important for character spacing; 0 < width <= pixelfont_header.resolution
thickness: important info for drawing bold characters; 0 < thickness <= pixelfont_header.resolution

array of length pixelfont_header.bitmap_count

## Bitmap data section

stores a uint32_t[pixelfont_header.resolution * pixelfont_header.resolution / 32] array of length pixelfont_header.bitmap_count

Stores the bitmap data in row mayor order from the top left



# Generation

pixelfont_config.yaml


