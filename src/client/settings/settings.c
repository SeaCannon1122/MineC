#include "client/game_client.h"

uint32_t settings_load(struct game_client* game) {

	uint64_t scale_buffer;
	key_value_get_integer(&game->resource_state.key_value_map_atlas[KVM_SETTINGS], "gui_scale", 2, &scale_buffer);
	game->settings_state.game_settings.gui_scale = scale_buffer;

	return 0;
}

uint32_t settings_save(struct game_client* game) {

	resource_manager_save_key_value_map(&game->resource_state.resource_manager, resources_key_value_map_tokens[KVM_SETTINGS], game->resource_state.key_value_map_atlas[KVM_SETTINGS]);

	return 0;
} 