#pragma once

enum block_type {
	BLOCK_AIR = 0,
	BLOCK_DIRT = 1,
	BLOCK_GRASS = 2,
	BLOCK_STONE = 3,
	BLOCK
};

struct game_raw_chunk {
	struct {
		short block_id;
		char variant;
		char light_level;
	} blocks[16 * 16 * 16];
};