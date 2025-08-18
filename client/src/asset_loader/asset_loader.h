#pragma once

#ifndef MINEC_CLIENT_ASSET_LOADER_ASSET_LOADER_H
#define MINEC_CLIENT_ASSET_LOADER_ASSET_LOADER_H

#include <stdint.h>
#include <mutex.h>

#define ASSET_LOADER client->asset_loader

struct minec_client;

struct asset_loader
{
	mutex_t mutex;
	void* alloc;

	void* resourcepack_paths_arraylist;
};

#ifndef MINEC_CLIENT_INCLUDE_ONLY_STRUCTURE

struct minec_client;

uint32_t asset_loader_create(struct minec_client* client);
void asset_loader_destroy(struct minec_client* client);

uint32_t asset_loader_sync_with_settings(struct minec_client* client);

void* asset_loader_asset_load(struct minec_client* client, uint8_t* name, uint8_t** data, size_t* size);
void asset_loader_asset_unload(struct minec_client* client, void* asset_handle);

#endif

#endif