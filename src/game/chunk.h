#pragma once

enum block_type {
	BLOCK_AIR = 0,
	BLOCK_DIRT = 1,
	BLOCK_GRASS = 2,
	BLOCK_STONE = 3,
	BLOCK
};

struct block_type {
	unsigned short id;
	unsigned short variant;
};

struct raw_chunk {
	struct {
		struct block_type type;
	} blocks[16 * 16 * 16];
};