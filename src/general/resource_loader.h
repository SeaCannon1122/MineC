#pragma once

#ifndef RESOURCE_LOADER_H
#define RESOURCE_LOADER_H

#define RESOURCE_LOADER_MAX_GROUPS_PER_ITEM 8

#include <stdint.h>

void* resource_loader_create();

void resource_loader_destroy(void* rl);

#endif // !RESOURCE_LOADER_H
