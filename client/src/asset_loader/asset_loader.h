#pragma once

#ifndef MINEC_CLIENT_ASSET_LOADER_ASSET_LOADER_H
#define MINEC_CLIENT_ASSET_LOADER_ASSET_LOADER_H

#include <stdint.h>
#include <stdbool.h>
#include <mutex.h>

#define ASSET_LOADER client->asset_loader

struct minec_client;

struct asset_loader_asset
{
	void* data;
	size_t size;
	bool is_external;
};

struct asset_loader
{
	mutex_t mutex;
	void* alloc;

	struct asset_loader_asset* assets;
	uint32_t asset_count;
	void* asset_names_hashmap;
	atomic_uint8_t borrowed_asset_count;
};

#ifndef MINEC_CLIENT_INCLUDE_ONLY_STRUCTURE

struct minec_client;

uint32_t asset_loader_create(struct minec_client* client);
void asset_loader_destroy(struct minec_client* client);

uint32_t asset_loader_reload(struct minec_client* client);

void* asset_loader_get_asset(struct minec_client* client, uint8_t* name, size_t* size);
void asset_loader_release_asset(struct minec_client* client);

#endif

#endif