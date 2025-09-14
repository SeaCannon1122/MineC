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
#include <arraylist.h>
#include <atomics.h>
#include <utils.h>
#include <stringutils.h>
#include <stb_image/stb_image.h>

#include "logging.h"
#include "resource_index.h"
#include "string_index/string_index.h"
#include "application_window/application_window.h"
#include "settings/settings.h"
#include "gui/gui.h"
#include "renderer/renderer.h"
#include "asset_loader/asset_loader.h"

enum minec_client_result
{
	MINEC_CLIENT_SUCCESS,
	MINEC_CLIENT_ERROR
};

struct minec_client
{
	uint8_t* data_files_path;
	size_t data_files_path_length;

	struct settings settings;
	struct string_index string_index;
	struct asset_loader asset_loader;
	struct application_window window;
	struct gui_state gui_state;
	struct renderer renderer;
};

#ifndef MINEC_CLIENT_INCLUDE_ONLY_STRUCTURE

void minec_client_reload_assets(struct minec_client* client);

void minec_client_run(uint8_t* data_files_path);

#endif

#endif