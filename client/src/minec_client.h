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
#include <cerialize.h>

#include "logging.h"
#include "application_window/application_window.h"
#include "settings/settings.h"
#include "renderer/renderer.h"

#ifndef MINEC_CLIENT_DYNAMIC_RENDERER_LIBRARY
#include "assets.h"
#endif

enum minec_client_result
{
	MINEC_CLIENT_SUCCESS,
	MINEC_CLIENT_ERROR
};

struct minec_client
{
	uint8_t* data_files_path;
	size_t data_files_path_length;

	struct application_window window;

	struct settings settings;

	struct renderer renderer;
};

void minec_client_run(uint8_t* data_files_path);

#endif