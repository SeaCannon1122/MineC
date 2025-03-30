#pragma once



struct game_raw_chunk {
	struct {
		uint8_t block_id;
	} blocks[16 * 16 * 16];
};