#include "gui.h"

#include <string.h>

#include "client/game_client.h"

enum gui_items {
	GUI_LABEL,
	GUI_BUTTON,
	GUI_BUTTON_SMALL,
	GUI_SLIDER,
	GUI_CHECKBOX,
	GUI_IMAGE,
	GUI_TEXTFIELD,
};

struct _gui_scene_header {
	uint32_t item_count;
	uint32_t item_index;
	uint32_t labeltext_buffer_length;
	uint32_t labeltext_buffer_index;
	uint32_t textfield_buffer_size;
	uint32_t textfield_buffer_index;
};

struct _gui_item {
	float x;
	float y;
	uint32_t offset_x;
	uint32_t offset_y;
	float alignment_x;
	float alignment_y;
	uint32_t z;
	uint32_t gui_item_type;
	union {
		struct {
			uint32_t text_buffer_index;
			uint32_t text_length;
			uint32_t max_text_length;
			uint32_t text_size;
		} label;
		struct {
			uint32_t disabled;
			uint32_t clicked;
		} button;
		struct {
			uint32_t disabled;
			uint32_t clicked;
		} button_small;
		struct {
			float value;
		} slider;
		struct {
			uint32_t checked;
		} checkbox;
		struct {
			uint32_t image;
		} image;
		struct {
			uint32_t index;
			uint32_t size;
			uint32_t max_size;
		} text_field;
	} item_info;
};

void* gui_scene_new(uint32_t item_count, uint32_t total_labeltext_buffer_length, uint32_t total_textfield_buffer_size) {

	struct _gui_scene_header* scene = malloc(sizeof(struct _gui_scene_header) + sizeof(struct _gui_item) * item_count + sizeof(struct game_char) * total_labeltext_buffer_length + total_textfield_buffer_size * sizeof(uint8_t));
	scene->item_count = item_count;
	scene->textfield_buffer_index = 0;
	scene->labeltext_buffer_length = total_labeltext_buffer_length;
	scene->labeltext_buffer_index = 0;
	scene->textfield_buffer_size = total_textfield_buffer_size;
	scene->textfield_buffer_index = 0;

	return scene;
}

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
) {
	
	struct _gui_scene_header* header = scene;

	if ((uint32_t)item_handle >= header->item_count) return 1;

	struct _gui_item* items = (size_t)scene + sizeof(struct _gui_scene_header);

	items[(uint32_t)item_handle].x = x;
	items[(uint32_t)item_handle].y = y;
	items[(uint32_t)item_handle].offset_x = offset_x;
	items[(uint32_t)item_handle].offset_y = offset_y;
	items[(uint32_t)item_handle].alignment_x = alignment_x;
	items[(uint32_t)item_handle].alignment_y = alignment_y;
	items[(uint32_t)item_handle].z = z;

	return 0;
}

void* gui_add_label(void* scene, uint32_t max_text_length) {

	struct _gui_scene_header* header = scene;
	
	if (header->item_index == header->item_count) return GUI_INVALID_ITEM_HANDLE;
	if (header->labeltext_buffer_index + max_text_length > header->labeltext_buffer_length) return GUI_INVALID_ITEM_HANDLE;

	uint32_t item_handle = header->item_index;

	struct _gui_item* items = (size_t)scene + sizeof(struct _gui_scene_header);

	items[item_handle].gui_item_type = GUI_LABEL;
	items[item_handle].item_info.label.max_text_length = max_text_length;
	items[item_handle].item_info.label.text_length = 0;
	items[item_handle].item_info.label.text_buffer_index = header->labeltext_buffer_index;


	header->labeltext_buffer_index += max_text_length;
	header->item_index++;

	return item_handle;
}

uint32_t gui_set_label(
	void* scene,
	void* label_handle,
	struct game_char* text,
	uint32_t text_length,
	uint32_t text_size
) {

	struct _gui_scene_header* header = scene;

	if ((uint32_t)label_handle >= header->item_count) return 1;

	struct _gui_item* items = (size_t)scene + sizeof(struct _gui_scene_header);

	if (items[(uint32_t)label_handle].item_info.label.max_text_length < text_length) return 1;

	items[(uint32_t)label_handle].item_info.label.text_length = text_length;
	items[(uint32_t)label_handle].item_info.label.text_size = text_size;

	memcpy(
		(size_t)scene + sizeof(struct _gui_scene_header) + (size_t)header->item_count * sizeof(struct _gui_item) + (size_t)items[(uint32_t)label_handle].item_info.label.text_buffer_index * sizeof(struct game_char),
		text,
		text_length * sizeof(struct game_char)
	);

	return 0;
}

//uint32_t gui_