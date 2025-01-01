#include "gui_menus.h"

#include "client/game_client.h"

uint32_t gui_menus_create(struct game_client* game) {

	uint8_t text[] = "This is a very long text.\n"
		"Well, at least if you compare it to what else i have written to this screen so far.\n"
		"Objectively speaking, this text is actually quite small but still large enough for it to not fit in a singular line!\n"
		"Hello World! WHat are yall doing on this fine evening? I'm just coding :) :=> ^-^\n"
		"rectangles[0].u[2] = (float)game->application_state.window_extent.width / (float)(4 * game->resource_state.image_atlas[IMAGE_MENU_BACKGROUND].width);\n"
		"https://www.youtube.com/watch?v=tLhfuc_KCpM\n"
		"Amphipoea oculea, the ear moth, is a moth of the family Noctuidae. It was first described by\n"
		"Carl Linnaeus in 1761 and it is found in most of the Palearctic realm.[citation needed] It is\n"
		"The wingspan is 29-34 mm. Forewing pale or dark ferruginous brown; the veins brown; inner and outer lines\n"
		"double, brown, wide apart; the inner curved outwards between, and toothed inwards on, the veins; the outer with the inner arm thin, lunulate-dentate, \n"
		"the outer thick, continuous and parallel; a thick dark median shade running between the \n"
		"stigmata; submarginal line indistinct, waved, angled on vein 7, above which it is preceded by a \n"
		"dark costal patch; orbicular stigma rounded, orange, with a brown ring; reniform white, with the\n"
		"veins across it brown and containing on the discocellular a brown-outlined lunule, of which the\n"
		"centre is yellowish; the colour with brown outline; hindwing fuscous grey, paler towards base; ";

	game->gui_menus_state.main.menu_handle = gui_scene_new(1, sizeof(text) - 1, 0);

	game->gui_menus_state.main.title_label_handle = gui_add_label(game->gui_menus_state.main.menu_handle, sizeof(text) - 1);

	gui_set_item_position(
		game->gui_menus_state.main.menu_handle,
		game->gui_menus_state.main.title_label_handle,
		0.5,
		0,
		0,
		40,
		0.5,
		0,
		0
	);

	

	struct game_char game_char_text[sizeof(text) - 1];

	for (uint32_t i = 0; i < sizeof(text) - 1; i++) {
		game_char_text[i].color[0] = 255;
		game_char_text[i].color[1] = 255;
		game_char_text[i].color[2] = 255;
		game_char_text[i].color[3] = 255;
		game_char_text[i].masks = PIXEL_CHAR_SHADOW_MASK | PIXELFONT_SMOOTH;
		game_char_text[i].value = text[i];
	}

	gui_set_label(
		game->gui_menus_state.main.menu_handle,
		game->gui_menus_state.main.title_label_handle,
		game_char_text,
		sizeof(text) - 1,
		1,
		4,
		0.5
	);

	game->gui_menus_state.active_menu = MENU_MAIN;

	return 0;
}

uint32_t gui_menus_destroy(struct game_client* game) {

	gui_scene_destroy(game->gui_menus_state.main.menu_handle);

	return 0;
}

uint32_t gui_menus_simulation_frame(struct game_client* game) {

	switch (game->gui_menus_state.active_menu) {

	case MENU_MAIN: {



	} break;

	}

}

uint32_t gui_menus_render(struct game_client* game) {

	switch (game->gui_menus_state.active_menu) {

	case MENU_MAIN: {

		gui_scene_render(game, game->gui_menus_state.main.menu_handle, 2);

	} break;

	}

	return 0;
}