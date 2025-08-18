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
#include <stb_image/stb_image.h>

#include "logging.h"
#include "application_window/application_window.h"
#include "settings/settings.h"
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
	struct asset_loader asset_loader;
	struct application_window window;
	struct renderer renderer;
};

void minec_client_run(uint8_t* data_files_path);

#endif