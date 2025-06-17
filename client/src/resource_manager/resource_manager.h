#pragma once

#ifndef MINEC_CLIENT_RESOURCE_MANAGER_RESOURCE_MANAGER_H
#define MINEC_CLIENT_RESOURCE_MANAGER_RESOURCE_MANAGER_H

#include <stdint.h>

struct minec_client;

uint32_t resource_manager_create(struct minec_client* client);



void resource_manager_destroy(struct minec_client* client);

#endif
