#pragma once

#ifndef MINEC_CLIENT_ASSET_LOADER_ASSET_LOADER_H
#define MINEC_CLIENT_ASSET_LOADER_ASSET_LOADER_H

#include <stdint.h>
#include <stdbool.h>
#include <mutex.h>

#define ASSET_LOADER client->asset_loader

#define ASSET_LOADER_MAX_RESOURCE_PACKS 255

struct minec_client;

struct asset_loader_asset
{
	void* data;
	size_t size;
	bool external;
};

struct asset_loader
{
	void* ressourcepack_path_arraylist;
	void* mapping_hashmaps[ASSET_LOADER_MAX_RESOURCE_PACKS + 1];
};

#ifndef MINEC_CLIENT_INCLUDE_ONLY_STRUCTURE

struct minec_client;

uint32_t asset_loader_create(struct minec_client* client);
void asset_loader_destroy(struct minec_client* client);

bool asset_loader_get_asset(struct minec_client* client, uint8_t* name, struct asset_loader_asset* asset);

#endif

#endif