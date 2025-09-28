#pragma once

#ifndef MINEC_CLIENT_RESOURCE_INDEX_H
#define MINEC_CLIENT_RESOURCE_INDEX_H

#include <stdint.h>
#include <stddef.h>

const void* resource_index_query(uint8_t* path, size_t* size);

#endif
