#pragma once

#ifndef RESOURCES_H
#define RESOURCES_H

#include "general/resource_manager.h"

enum resources_key_value_maps {
	RESOURCES_KVM_PATHS,
};

static const uint8_t* const resources_key_value_map_tokens[] = {
	"paths"
};

#define RESOURCES_KVM_COUNT sizeof(resources_key_value_map_tokens) / sizeof(resources_key_value_map_tokens[0])


struct resource_state {

	uint8_t resource_manager_root_path[256];
	struct resource_manager resource_manager;

	//key value maps
	void* key_value_map_atlas[RESOURCES_KVM_COUNT];

};

struct minec_client;

uint32_t resources_create(struct minec_server* game, uint8_t* resource_path);

uint32_t resources_destroy(struct minec_server* game);

uint32_t resources_reload(struct minec_server* game);

#endif // !RESOURCES_H
