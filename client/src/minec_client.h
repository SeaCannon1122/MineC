#pragma once

#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <stdarg.h>

#include "string_allocator.h"
#include "hashmap.h"

#include "application_window/application_window.h"
#include "settings/settings.h"
#include "resources/resources.h"
#include "renderer/renderer.h"

struct minec_client
{
	void* string_allocator;
	uint8_t* runtime_files_path;
	size_t runtime_files_path_length;

	struct application_window main_window;
	
	struct settings_state settings_state;

	struct resources_state resources_state;

	struct renderer_state renderer_state;

};

void* minec_client_load_file(uint8_t* path, size_t* size);

uint32_t minec_client_run(struct minec_client* client, uint8_t* runtime_files_path);

#endif // !GAME_CLIENT_H