#pragma once

#ifndef MINEC_CLIENT_RENDERER_RENDERER_INTERNAL_H
#define MINEC_CLIENT_RENDERER_RENDERER_INTERNAL_H

#include <minec_client.h>

struct renderer_internal_state
{
	struct renderer_settings_state settings_state;
	struct renderer_info_state info_state;
};

#endif
