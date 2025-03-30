#include "client/game_client.h"

enum gui_items {
	GUI_LABEL,
	GUI_BUTTON,
	GUI_SLIDER,
	GUI_CHECKBOX,
	GUI_IMAGE,
	GUI_TEXTFIELD,
};

struct _gui_scene_header {
	uint32_t item_count;
	uint32_t item_index;
	int32_t item_selected_index;
	uint32_t labeltext_buffer_length;
	uint32_t labeltext_buffer_index;
	uint32_t textfield_buffer_size;
	uint32_t textfield_buffer_index;
};

struct _gui_item {
	float x;
	float y;
	int32_t offset_x;
	int32_t offset_y;
	float alignment_x;
	float alignment_y;
	uint16_t z;
	uint16_t visibility;
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
			uint32_t size;
		} button;
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
			uint16_t index;
			uint16_t size;
			uint16_t max_size;
			uint16_t cursor_index;
			double cursor_blinking_time_start;
		} text_field;
	} item_info;
};

void* gui_scene_new(uint32_t item_count, uint32_t total_labeltext_buffer_length, uint32_t total_textfield_buffer_size) {

	struct _gui_scene_header* scene = malloc(sizeof(struct _gui_scene_header) + sizeof(struct _gui_item) * item_count + sizeof(struct game_char) * total_labeltext_buffer_length + (total_textfield_buffer_size + item_count) * sizeof(uint32_t));
	scene->item_count = item_count;
	scene->item_index = 0;
	scene->item_selected_index = -1;
	scene->labeltext_buffer_length = total_labeltext_buffer_length;
	scene->labeltext_buffer_index = 0;
	scene->textfield_buffer_size = total_textfield_buffer_size + item_count;
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
	int32_t offset_x,
	int32_t offset_y,
	float alignment_x,
	float alignment_y,
	uint16_t z
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
	items[(uint32_t)item_handle].visibility = 1;

	return 0;
}

uint32_t gui_set_item_visibility(void* scene, void* item_handle, uint16_t visibility) {

	struct _gui_scene_header* header = scene;
	if ((uint32_t)item_handle >= header->item_count) return 1;

	struct _gui_item* items = (size_t)scene + sizeof(struct _gui_scene_header);
	items[(uint32_t)item_handle].visibility = visibility;

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

void* gui_add_button(void* scene, uint32_t button_size) {
	struct _gui_scene_header* header = scene;

	if (header->item_index == header->item_count) return GUI_INVALID_ITEM_HANDLE;

	uint32_t item_handle = header->item_index;

	struct _gui_item* items = (size_t)scene + sizeof(struct _gui_scene_header);

	items[item_handle].gui_item_type = GUI_BUTTON;
	items[item_handle].item_info.button.disabled = 0;
	items[item_handle].item_info.button.clicked = 0;
	items[item_handle].item_info.button.size = button_size;

	header->item_index++;

	return item_handle;
}

uint32_t gui_enable_disable_button(void* scene, void* button_handle, uint32_t disabled) {

	struct _gui_scene_header* header = scene;

	if ((uint32_t)button_handle >= header->item_count) return 1;

	struct _gui_item* items = (size_t)scene + sizeof(struct _gui_scene_header);

	items[(uint32_t)button_handle].item_info.button.disabled = disabled;

	return 0;
}

uint32_t gui_is_button_clicked(void* scene, void* button_handle) {

	struct _gui_scene_header* header = scene;

	if ((uint32_t)button_handle >= header->item_count) return 0;

	struct _gui_item* items = (size_t)scene + sizeof(struct _gui_scene_header);

	if (items[(uint32_t)button_handle].item_info.button.disabled) return 0;

	return items[(uint32_t)button_handle].item_info.button.clicked;
}

void* gui_add_textfield(void* scene, uint16_t buffer_size, uint32_t** buffer_pointer) {

	struct _gui_scene_header* header = scene;

	if (header->item_index == header->item_count) return GUI_INVALID_ITEM_HANDLE;
	if (header->textfield_buffer_index + buffer_size + 1 > header->textfield_buffer_size) return GUI_INVALID_ITEM_HANDLE;

	uint32_t item_handle = header->item_index;

	struct _gui_item* items = (size_t)scene + sizeof(struct _gui_scene_header);

	items[item_handle].gui_item_type = GUI_TEXTFIELD;
	items[item_handle].item_info.text_field.max_size = buffer_size;
	items[item_handle].item_info.text_field.size = 0;
	items[item_handle].item_info.text_field.index = header->textfield_buffer_index;
	items[item_handle].item_info.text_field.cursor_index = 0;

	uint32_t* buffer_ptr = (size_t)scene + sizeof(struct _gui_scene_header) + (size_t)header->item_count * sizeof(struct _gui_item) + (size_t)header->labeltext_buffer_length * sizeof(struct game_char) + (size_t)header->textfield_buffer_index * sizeof(uint32_t);

	memset(
		buffer_ptr,
		0,
		buffer_size + 1
	);

	header->textfield_buffer_index += buffer_size + 1;
	header->item_index++;

	*buffer_pointer = buffer_ptr;

	return item_handle;
}

uint32_t gui_set_textfield_text(void* scene, void* textfield_handle, uint32_t* text, uint32_t text_length) {

	struct _gui_scene_header* header = scene;

	if ((uint32_t)textfield_handle >= header->item_count) return 1;

	struct _gui_item* items = (size_t)scene + sizeof(struct _gui_scene_header);

	if (items[(uint32_t)textfield_handle].item_info.text_field.max_size < text_length) return 1;

	items[(uint32_t)textfield_handle].item_info.text_field.size = text_length;

	uint32_t* buffer_ptr = (size_t)scene + sizeof(struct _gui_scene_header) + (size_t)header->item_count * sizeof(struct _gui_item) + (size_t)header->labeltext_buffer_length * sizeof(struct game_char) + (size_t)items[(uint32_t)textfield_handle].item_info.text_field.index * sizeof(uint32_t);

	memcpy(buffer_ptr, text, text_length * sizeof(uint32_t));
	buffer_ptr[text_length] = 0;

	items[(uint32_t)textfield_handle].item_info.text_field.cursor_index = text_length;

	return 0;
}

uint32_t gui_scene_simulate(struct game_client* game, void* scene, uint32_t scale) {

	struct _gui_scene_header* header = scene;

	struct _gui_item* items = (size_t)scene + sizeof(struct _gui_scene_header);

	for (uint32_t i = 0; i < header->item_index; i++) {

		if (items[i].visibility == 0) continue;

		float anchor_x = items[i].x * (float)game->application_state.window_extent.width + (float)(items[i].offset_x * (int32_t)scale);
		float anchor_y = items[i].y * (float)game->application_state.window_extent.height + (float)(items[i].offset_y * (int32_t)scale);

		switch (items[i].gui_item_type) {
		
		case GUI_BUTTON: {

			items[i].item_info.button.clicked = 0;

			if (items[i].item_info.button.disabled || game->application_state.input_state.keyboard[KEY_MOUSE_LEFT] != 0b11) continue;

			uint32_t button_width = game->resource_state.image_atlas[items[i].item_info.button.size == GUI_SIZE_NORMAL ? IMAGE_MENU_BUTTON : IMAGE_MENU_BUTTON_SHORT].width;
			uint32_t button_height = game->resource_state.image_atlas[items[i].item_info.button.size == GUI_SIZE_NORMAL ? IMAGE_MENU_BUTTON : IMAGE_MENU_BUTTON_SHORT].height;

			int32_t x0 = (int32_t)(anchor_x)-(int32_t)(items[i].alignment_x * (float)button_width) * (int32_t)scale;
			int32_t x1 = x0 + button_width * scale;

			int32_t y0 = (int32_t)(anchor_y)-(int32_t)(items[i].alignment_y * (float)button_height) * (int32_t)scale;
			int32_t y1 = y0 + button_height * scale;

			if (
				game->application_state.input_state.mouse_coords.x >= x0 &&
				game->application_state.input_state.mouse_coords.x < x1 &&
				game->application_state.input_state.mouse_coords.y >= y0 &&
				game->application_state.input_state.mouse_coords.y < y1
			) items[i].item_info.button.clicked = 1;

		} break;
		
		case GUI_TEXTFIELD: {

			int32_t x0 = (int32_t)(anchor_x)-(int32_t)(items[i].alignment_x * (float)game->resource_state.image_atlas[IMAGE_MENU_TEXTFIELD].width) * (int32_t)scale;
			int32_t x1 = x0 + game->resource_state.image_atlas[IMAGE_MENU_TEXTFIELD].width * scale;

			int32_t y0 = (int32_t)(anchor_y)-(int32_t)(items[i].alignment_y * (float)game->resource_state.image_atlas[IMAGE_MENU_TEXTFIELD].height) * (int32_t)scale;
			int32_t y1 = y0 + game->resource_state.image_atlas[IMAGE_MENU_TEXTFIELD].height * scale;

			if (
				game->application_state.input_state.mouse_coords.x >= x0 &&
				game->application_state.input_state.mouse_coords.x < x1 &&
				game->application_state.input_state.mouse_coords.y >= y0 &&
				game->application_state.input_state.mouse_coords.y < y1 &&
				game->application_state.input_state.keyboard[KEY_MOUSE_LEFT] == 0b11
			) {
				if (header->item_selected_index != i) items[i].item_info.text_field.cursor_blinking_time_start = game->application_state.time;
				header->item_selected_index = i;
			}

			else if (header->item_selected_index == i) {

				if (items[i].item_info.text_field.size > items[i].item_info.text_field.cursor_index && game->application_state.input_state.keyboard[KEY_ARROW_RIGHT] == 0b11) {
					items[i].item_info.text_field.cursor_index++;
					items[i].item_info.text_field.cursor_blinking_time_start = game->application_state.time;
				}
				if (0 < items[i].item_info.text_field.cursor_index && game->application_state.input_state.keyboard[KEY_ARROW_LEFT] == 0b11) {
					items[i].item_info.text_field.cursor_index--;
					items[i].item_info.text_field.cursor_blinking_time_start = game->application_state.time;
				}

				uint32_t* buffer_ptr = (size_t)scene + sizeof(struct _gui_scene_header) + (size_t)header->item_count * sizeof(struct _gui_item) + (size_t)header->labeltext_buffer_length * sizeof(struct game_char) + (size_t)items[i].item_info.text_field.index * sizeof(uint32_t);

				for (uint32_t j = 0; j < game->application_state.input_state.character_count; j++) {

					if (game->application_state.input_state.characters[j] == (uint32_t)'\b') {

						if (items[i].item_info.text_field.cursor_index > 0) {

							memmove(&buffer_ptr[items[i].item_info.text_field.cursor_index - 1], &buffer_ptr[items[i].item_info.text_field.cursor_index], (items[i].item_info.text_field.size - items[i].item_info.text_field.cursor_index + 1) * sizeof(uint32_t));
							items[i].item_info.text_field.cursor_index--;
							items[i].item_info.text_field.size--;
							items[i].item_info.text_field.cursor_blinking_time_start = game->application_state.time;
						}
					}

					else if(items[i].item_info.text_field.size < items[i].item_info.text_field.max_size) {

						memmove(&buffer_ptr[items[i].item_info.text_field.cursor_index + 1], &buffer_ptr[items[i].item_info.text_field.cursor_index], (items[i].item_info.text_field.size - items[i].item_info.text_field.cursor_index + 1) * sizeof(uint32_t));
						buffer_ptr[items[i].item_info.text_field.cursor_index] = game->application_state.input_state.characters[j];
						items[i].item_info.text_field.cursor_index++;
						items[i].item_info.text_field.size++;
						items[i].item_info.text_field.cursor_blinking_time_start = game->application_state.time;
					}
				}

			}


		} break;

		}

	}

	return 0;
}

uint32_t gui_scene_render(struct game_client* game, void* scene, uint32_t scale) {

	struct _gui_scene_header* header = scene;

	struct _gui_item* items = (size_t)scene + sizeof(struct _gui_scene_header);

	for (uint32_t i = 0; i < header->item_index; i++) {

		if (items[i].visibility == 0) continue;

		float anchor_x = items[i].x * (float)game->application_state.window_extent.width  + (float)(items[i].offset_x * (int32_t)scale);
		float anchor_y = items[i].y * (float)game->application_state.window_extent.height + (float)(items[i].offset_y * (int32_t)scale);

		switch (items[i].gui_item_type) {
		
		case GUI_LABEL: {

			struct game_char* text = (size_t)scene + sizeof(struct _gui_scene_header) + (size_t)header->item_count * sizeof(struct _gui_item) + (size_t)items[i].item_info.label.text_buffer_index * sizeof(struct game_char);

			uint32_t text_size = items[i].item_info.label.text_size * scale;


			uint32_t lines_count = 1;

			for (uint32_t char_i = 0; char_i < items[i].item_info.label.text_length; char_i++) if (text[char_i].value == '\n') lines_count++;

			uint32_t* line_widths = alloca(sizeof(uint32_t) * lines_count);

			uint32_t line_i = 0;
			line_widths[0] = 0;

			for (uint32_t char_i = 0; char_i < items[i].item_info.label.text_length; char_i++) {

				if (text[char_i].value == '\n') {
					line_i++;
					line_widths[line_i] = 0;
				}
				else line_widths[line_i] += ((game->resource_state.pixelfont_atlas[text[char_i].masks & PIXELCHAR_MASK_FONT]->bitmaps[text[char_i].value].width + 1) / 2 + 1) * text_size;

			}

			uint32_t label_height = (lines_count * (8 + items[i].item_info.label.line_spacing) - items[i].item_info.label.line_spacing) * text_size;
			uint32_t label_width = 0;

			for (uint32_t line_i = 0; line_i < lines_count; line_i++) {
				if (line_widths[line_i] != 0) line_widths[line_i] -= text_size;
				if (line_widths[line_i] > label_width) label_width = line_widths[line_i];
			}

			uint32_t char_i = 0;

			int32_t y = (int32_t)(anchor_y - items[i].alignment_y * (float)label_height);

			for (uint32_t line_i = 0; ; line_i++) {

				if (char_i >= items[i].item_info.label.text_length) break;

				int32_t x = (int32_t)(anchor_x - items[i].alignment_x * (float)label_width + items[i].item_info.label.text_alignment * (float)(label_width - line_widths[line_i]));

				for (; char_i < items[i].item_info.label.text_length; char_i++) {
					if (text[char_i].value == '\n') {
						char_i++;
						break;
					}

					struct pixelchar pc;

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

					x += ((game->resource_state.pixelfont_atlas[text[char_i].masks & PIXELCHAR_MASK_FONT]->bitmaps[text[char_i].value].width + 1) / 2 + 1) * text_size;

				}
			
				y += text_size * (8 + items[i].item_info.label.line_spacing);
			}


		} break;

		case GUI_BUTTON: {

			uint32_t button_size = items[i].item_info.button.size;

			uint32_t button_width = game->resource_state.image_atlas[button_size == GUI_SIZE_NORMAL ? IMAGE_MENU_BUTTON : IMAGE_MENU_BUTTON_SHORT].width;
			uint32_t button_height = game->resource_state.image_atlas[button_size == GUI_SIZE_NORMAL ? IMAGE_MENU_BUTTON : IMAGE_MENU_BUTTON_SHORT].height;

			struct renderer_rectangle rect;
			rect.sampler_index = SAMPLER_DEFAULT;

			rect.u[0] = 0.f;
			rect.u[1] = 0.f;
			rect.u[2] = 1.f;
			rect.u[3] = 1.f;

			rect.v[0] = 0.f;
			rect.v[1] = 1.f;
			rect.v[2] = 1.f;
			rect.v[3] = 0.f;

			rect.x[0] = (int32_t)(anchor_x) - (int32_t)(items[i].alignment_x * (float)button_width) * (int32_t)scale;
			rect.x[1] = rect.x[0];
			rect.x[2] = rect.x[0] + button_width * scale;
			rect.x[3] = rect.x[2];

			rect.y[0] = (int32_t)(anchor_y) - (int32_t)(items[i].alignment_y * (float)button_height) * (int32_t)scale;
			rect.y[1] = rect.y[0] + button_height * scale;
			rect.y[2] = rect.y[1];
			rect.y[3] = rect.y[0];

			if (items[i].item_info.button.disabled) rect.image_index = (button_size == GUI_SIZE_NORMAL ? IMAGE_MENU_BUTTON_DISABLED : IMAGE_MENU_BUTTON_DISABLED_SHORT);

			else if (
				game->application_state.input_state.mouse_coords.x >= rect.x[0] &&
				game->application_state.input_state.mouse_coords.x < rect.x[2] &&
				game->application_state.input_state.mouse_coords.y >= rect.y[0] &&
				game->application_state.input_state.mouse_coords.y < rect.y[1]
			) rect.image_index = (button_size == GUI_SIZE_NORMAL ? IMAGE_MENU_BUTTON_HOVER : IMAGE_MENU_BUTTON_HOVER_SHORT);

			else rect.image_index = (button_size == GUI_SIZE_NORMAL ? IMAGE_MENU_BUTTON : IMAGE_MENU_BUTTON_SHORT);

			renderer_backend_add_rectangles(game, &rect, 1);

		} break;

		case GUI_TEXTFIELD: {

			struct renderer_rectangle rect;
			rect.sampler_index = SAMPLER_DEFAULT;

			rect.u[0] = 0.f;
			rect.u[1] = 0.f;
			rect.u[2] = 1.f;
			rect.u[3] = 1.f;

			rect.v[0] = 0.f;
			rect.v[1] = 1.f;
			rect.v[2] = 1.f;
			rect.v[3] = 0.f;

			rect.x[0] = (int16_t)(anchor_x) - (int16_t)(items[i].alignment_x * (float)game->resource_state.image_atlas[IMAGE_MENU_TEXTFIELD].width) * (int16_t)scale;
			rect.x[1] = rect.x[0];
			rect.x[2] = rect.x[0] + game->resource_state.image_atlas[IMAGE_MENU_TEXTFIELD].width * scale;
			rect.x[3] = rect.x[2];

			rect.y[0] = (int32_t)(anchor_y) - (int32_t)(items[i].alignment_y * (float)game->resource_state.image_atlas[IMAGE_MENU_TEXTFIELD].height) * (int16_t)scale;
			rect.y[1] = rect.y[0] + game->resource_state.image_atlas[IMAGE_MENU_TEXTFIELD].height * scale;
			rect.y[2] = rect.y[1];
			rect.y[3] = rect.y[0];

			if (i == header->item_selected_index) rect.image_index = IMAGE_MENU_TEXTFIELD_SELECTED;
			else rect.image_index = IMAGE_MENU_TEXTFIELD;

			renderer_backend_add_rectangles(game, &rect, 1);

			struct pixelchar pc;

			pc.color[0] = 255;
			pc.color[1] = 255;
			pc.color[2] = 255;
			pc.color[3] = 255;

			pc.masks = PIXELCHAR_MASK_SHADOW;

			pc.size = scale;

			pc.position[0] = rect.x[0] + (game->resource_state.image_atlas[IMAGE_MENU_TEXTFIELD].height - 8) / 4 * (int16_t)scale;
			pc.position[1] = rect.y[0] + (game->resource_state.image_atlas[IMAGE_MENU_TEXTFIELD].height - 8) / 2 * (int16_t)scale;

			uint32_t* buffer_ptr = (size_t)scene + sizeof(struct _gui_scene_header) + (size_t)header->item_count * sizeof(struct _gui_item) + (size_t)header->labeltext_buffer_length * sizeof(struct game_char) + (size_t)items[i].item_info.text_field.index * sizeof(uint32_t);

			for (uint32_t j = 0; j < items[i].item_info.text_field.size; j++) {

				if (j == items[i].item_info.text_field.cursor_index && header->item_selected_index == i && fmod(game->application_state.time - items[i].item_info.text_field.cursor_blinking_time_start, 1400.0) < 700.0) {
					pc.position[0] -= scale;

					pc.color[0] = 150;
					pc.color[1] = 150;
					pc.color[2] = 150;

					pc.value = (uint32_t)'|';
					renderer_backend_add_pixel_chars(game, &pc, 1);

					pc.color[0] = 255;
					pc.color[1] = 255;
					pc.color[2] = 255;

					pc.position[0] += scale;
				}

				pc.value = buffer_ptr[j];

				renderer_backend_add_pixel_chars(game, &pc, 1);

				pc.position[0] += ((game->resource_state.pixelfont_atlas[PIXELFONT_DEFAULT]->bitmaps[buffer_ptr[j]].width + 1) / 2 + 1) * scale;
			}

			if (items[i].item_info.text_field.size == items[i].item_info.text_field.cursor_index && header->item_selected_index == i && fmod(game->application_state.time - items[i].item_info.text_field.cursor_blinking_time_start, 1400.0) < 900.0) {
				pc.value = (uint32_t)'_';

				pc.color[0] = 150;
				pc.color[1] = 150;
				pc.color[2] = 150;

				renderer_backend_add_pixel_chars(game, &pc, 1);
			}

		} break;

		}

	}

	return 0;
}