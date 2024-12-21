#ifndef GUI_H
#define GUI_H

#include <stdint.h>

#include "general/rendering/gui/pixel_char.h"

#include "menus.h"

struct gui_state {

	struct menus_state menus_state;

	struct pixel_char_renderer pixel_char_renderer;

};

uint32_t gui_create(struct game_client* game);

uint32_t gui_use_device(struct game_client* game);

uint32_t gui_drop_device(struct game_client* game);

uint32_t gui_destroy(struct game_client* game);

#endif