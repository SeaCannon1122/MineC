#include "gui.h"

#include <string.h>
#include <malloc.h>

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
			uint16_t text_length;
			uint16_t max_text_length;
			uint16_t text_size;
			uint16_t line_spacing;
			float text_alignment;
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
	scene->item_index = 0;
	scene->labeltext_buffer_length = total_labeltext_buffer_length;
	scene->labeltext_buffer_index = 0;
	scene->textfield_buffer_size = total_textfield_buffer_size;
	scene->textfield_buffer_index = 0;

	return scene;
}

uint32_t gui_scene_destroy(void* scene) {

	free(scene);

	return 0;
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

void* gui_add_label(void* scene, uint16_t max_text_length) {

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
	uint16_t text_length,
	uint16_t text_size,
	uint16_t line_spacing,
	float text_alignment
) {

	struct _gui_scene_header* header = scene;

	if ((uint32_t)label_handle >= header->item_count) return 1;

	struct _gui_item* items = (size_t)scene + sizeof(struct _gui_scene_header);

	if (items[(uint32_t)label_handle].item_info.label.max_text_length < text_length) return 1;

	items[(uint32_t)label_handle].item_info.label.text_length = text_length;
	items[(uint32_t)label_handle].item_info.label.text_size = text_size;
	items[(uint32_t)label_handle].item_info.label.line_spacing = line_spacing;
	items[(uint32_t)label_handle].item_info.label.text_alignment = text_alignment;

	memcpy(
		(size_t)scene + sizeof(struct _gui_scene_header) + (size_t)header->item_count * sizeof(struct _gui_item) + (size_t)items[(uint32_t)label_handle].item_info.label.text_buffer_index * sizeof(struct game_char),
		text,
		text_length * sizeof(struct game_char)
	);

	return 0;
}

uint32_t compare_item_z(struct _gui_item** item_a, struct _gui_item** item_b) {

	return (**item_a).z - (**item_b).z;

}

uint32_t gui_scene_render(struct game_client* game, void* scene, uint32_t scale) {

	struct _gui_scene_header* header = scene;

	struct _gui_item* items = (size_t)scene + sizeof(struct _gui_scene_header);

	struct _gui_item** ordered_items =  alloca(sizeof(struct _gui_item*) * header->item_index);

	for (uint32_t i = 0; i < header->item_index; i++) ordered_items[i] = &items[i];

	qsort(ordered_items, header->item_index, sizeof(struct _gui_item*), compare_item_z);

	for (uint32_t i = 0; i < header->item_index; i++) {

		float anchor_x = ordered_items[i]->x * (float)game->application_state.window_extent.width  + (float)(ordered_items[i]->offset_x * scale);
		float anchor_y = ordered_items[i]->y * (float)game->application_state.window_extent.height + (float)(ordered_items[i]->offset_y * scale);

		switch (ordered_items[i]->gui_item_type) {
		
		case GUI_LABEL: {

			struct game_char* text = (size_t)scene + sizeof(struct _gui_scene_header) + (size_t)header->item_count * sizeof(struct _gui_item) + (size_t)ordered_items[i]->item_info.label.text_buffer_index * sizeof(struct game_char);

			uint32_t text_size = ordered_items[i]->item_info.label.text_size * scale;


			uint32_t lines_count = 1;

			for (uint32_t char_i = 0; char_i < ordered_items[i]->item_info.label.text_length; char_i++) if (text[char_i].value == '\n') lines_count++;

			uint32_t* line_widths = alloca(sizeof(uint32_t) * lines_count);

			uint32_t line_i = 0;
			line_widths[0] = 0;

			for (uint32_t char_i = 0; char_i < ordered_items[i]->item_info.label.text_length; char_i++) {

				if (text[char_i].value == '\n') {
					line_i++;
					line_widths[line_i] = 0;
				}
				else line_widths[line_i] += (game->resource_state.pixelfont_atlas[text[char_i].masks & PIXEL_CHAR_FONT_MASK]->char_font_entries[text[char_i].value].width + 1) * text_size;

			}

			uint32_t label_height = (lines_count * (8 + ordered_items[i]->item_info.label.line_spacing) - ordered_items[i]->item_info.label.line_spacing) * text_size;
			uint32_t label_width = 0;

			for (uint32_t line_i = 0; line_i < lines_count; line_i++) {
				if (line_widths[line_i] != 0) line_widths[line_i] -= text_size;
				if (line_widths[line_i] > label_width) label_width = line_widths[line_i];
			}

			uint32_t char_i = 0;

			uint32_t y = (uint32_t)(anchor_y - ordered_items[i]->alignment_y * (float)label_height) + ordered_items[i]->offset_y;

			for (uint32_t line_i = 0; ; line_i++) {

				if (char_i >= ordered_items[i]->item_info.label.text_length) break;

				y += text_size * (8 + ordered_items[i]->item_info.label.line_spacing);

				uint32_t x = (uint32_t)(anchor_x - ordered_items[i]->alignment_x * (float)label_width + ordered_items[i]->item_info.label.text_alignment * (float)(label_width - line_widths[line_i])) + ordered_items[i]->offset_x;

				for (; char_i < ordered_items[i]->item_info.label.text_length; char_i++) {
					if (text[char_i].value == '\n') {
						char_i++;
						break;
					}

					struct pixel_char pc;

					pc.background_color[0] = text[char_i].background_color[0];
					pc.background_color[1] = text[char_i].background_color[1];
					pc.background_color[2] = text[char_i].background_color[2];
					pc.background_color[3] = text[char_i].background_color[3];

					pc.color[0] = text[char_i].color[0];
					pc.color[1] = text[char_i].color[1];
					pc.color[2] = text[char_i].color[2];
					pc.color[3] = text[char_i].color[3];

					pc.masks = text[char_i].masks;

					pc.value = text[char_i].value;

					pc.size = text_size;

					pc.position[0] = x;
					pc.position[1] = y;

					renderer_backend_add_pixel_chars(game, &pc, 1);

					x += ((game->resource_state.pixelfont_atlas[text[char_i].masks & PIXEL_CHAR_FONT_MASK]->char_font_entries[text[char_i].value].width + 1) / 2 + 1) * text_size;

				}
				
			}


		} break;

		}

	}

	return 0;
}