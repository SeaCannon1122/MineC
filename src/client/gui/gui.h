#pragma once

#ifndef GUI_H
#define GUI_H

#include <stdint.h>

#include "game/game.h"

#define GUI_INVALID_ITEM_HANDLE (void*)(uint32_t)(-1)

#define GUI_SIZE_NORMAL 0
#define GUI_SIZE_SHORT 1


void* gui_scene_new(uint32_t item_count, uint32_t total_labeltext_buffer_length, uint32_t total_textfield_buffer_size);
uint32_t gui_scene_destroy(void* scene);

uint32_t gui_set_item_position(
	void* scene,
	void* item_handle,
	float x,
	float y,
	int32_t offset_x,
	int32_t offset_y,
	float alignment_x,
	float alignment_y,
	uint16_t z
);

uint32_t gui_set_item_visibility(void* scene, void* item_handle, uint16_t visibility);

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

void* gui_add_button(void* scene, uint32_t button_size);
uint32_t gui_enable_disable_button(void* scene, void* button_handle, uint32_t disabled);
uint32_t gui_is_button_clicked(void* scene, void* button_handle);

void* gui_add_textfield(void* scene, uint16_t buffer_size, uint32_t** buffer_pointer);
uint32_t gui_set_textfield_text(void* scene, void* textfield_handle, uint32_t* text, uint32_t text_length);

struct game_client;

uint32_t gui_scene_simulate(struct game_client* game, void* scene, uint32_t scale);

uint32_t gui_scene_render(struct game_client* game, void* scene, uint32_t scale);

#endif