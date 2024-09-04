#pragma once

#include "keyvalue.h"

enum resource_type {
	RESOURCE_ARGB_IMAGE = 0,
	RESOURCE_PIXELFONT = 1,
	RESOURCE_KEYVALUE = 2,
	RESOURCE_TEXT_STRING = 3,
};

struct resource_manager_entry {
	char resource_token[MAX_KEY_SIZE];
	char resource_type;
	void* data;
};

extern void* resource_manager;

void resource_manager_init();

void* get_resource(char* token);

void resource_manager_exit();