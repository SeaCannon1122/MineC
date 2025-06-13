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
#include <cerialize.h>

#include "logging.h"
#include "application_window/application_window.h"
#include "settings/settings.h"
#include "resources/resources.h"
#include "renderer/renderer.h"
#include "assets.h"

enum minec_client_result
{
	MINEC_CLIENT_SUCCESS,
	MINEC_CLIENT_ERROR
};

struct minec_client
{
	void* static_alloc;
	void* dynamic_alloc;
	uint8_t* runtime_files_path;
	size_t runtime_files_path_length;

	struct application_window window;

	struct settings settings;

	struct resource_index resource_index;

	struct renderer renderer;
	atomic_(bool) renderer_recreate;
};

void minec_client_run(uint8_t* runtime_files_path);

//should not be called unless catastrophic unrecoverable operating system failure (NOT malloc failing)
static void minec_client_nuke_destroy_kill_crush_annihilate_process_exit(struct minec_client* client)
{
	exit(EXIT_FAILURE);
}

#endif