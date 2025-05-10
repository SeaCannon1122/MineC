#pragma once

#ifndef MINEC_CLIENT_H
#define MINEC_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <stdarg.h>

#include <string_allocator.h>
#include <hashmap.h>
#include <atomics.h>
#include <utils.h>

#include "logging.h"
#include "application_window/application_window.h"
#include "settings/settings.h"
#include "resources/resources.h"
#include "renderer/renderer.h"

enum minec_client_result
{
	MINEC_CLIENT_SUCCESS = 0,
	MINEC_CLIENT_ERROR,
	MINEC_CLIENT_ERROR_OUT_OF_MEMORY,
};

struct minec_client
{
	void* static_alloc;
	void* dynamic_alloc;
	uint8_t* runtime_files_path;
	size_t runtime_files_path_length;

	struct application_window window;
	
	struct settings settings;

	struct resources_index resources_index;

	struct renderer renderer;

};

void* minec_client_load_file(uint8_t* path, size_t* size);

uint32_t minec_client_run(struct minec_client* client, uint8_t* runtime_files_path);

#endif