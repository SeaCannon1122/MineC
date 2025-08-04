#pragma once

#ifndef MINEC_CLIENT_SETTINGS_SETTINGS_SETTINGS_H
#define MINEC_CLIENT_SETTINGS_SETTINGS_SETTINGS_H

#include <stdint.h>

#include <renderer/renderer.h>

struct settings {

	struct renderer_settings renderer;

	void* resource_pack_paths_hashmap;
};

struct minec_client;

void settings_create(struct minec_client* client);
void settings_destroy(struct minec_client* client);

void settings_load(struct minec_client* client);
void settings_save(struct minec_client* client);

#endif // !SETTINGS_H
