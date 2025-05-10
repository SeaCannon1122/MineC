#pragma once

#ifndef MINEC_CLIENT_RENDERER_BACKEND_BACKEND_INTERNAL_H
#define MINEC_CLIENT_RENDERER_BACKEND_BACKEND_INTERNAL_H

#include "backend.h"
#include <minec_client.h>

EXPORT struct renderer_backend_interface* renderer_backend_get_interfaces(void* window_context, uint32_t* count);

void* renderer_backend_get_window_context();

#endif
