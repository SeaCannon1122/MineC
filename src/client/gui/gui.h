#pragma once

#ifndef GUI_H
#define GUI_H

#include <stdint.h>

#include "game/game_char.h"


#define GUI_INVALID_ITEM_HANDLE (void*)(uint32_t)(-1)
  
void* gui_scene_new(uint32_t item_count, uint32_t total_labeltext_buffer_length, uint32_t total_textfield_buffer_size);
uint32_t gui_scene_destroy(void* scene);

uint32_t gui_set_item_position(
	void* scene,
	void* item_handle,
	float x,
	float y,
	uint32_t offset_x,
	uint32_t offset_y,
	float alignment_x,
	float alignment_y,
	uint32_t z
);

void* gui_add_label(void* scene, uint16_t max_text_length);

uint32_t gui_set_label(
	void* scene,
	void* label_handle,
	struct game_char* text,
	uint16_t text_length,
	uint16_t text_size,
	uint16_t line_spacing,
	float text_alignment
);

uint32_t gui_scene_render(struct game_client* game, void* scene, uint32_t scale);

#endif