#pragma once

#ifndef MINEC_CLIENT_RENDERER_RENDERER_H
#define MINEC_CLIENT_RENDERER_RENDERER_H

#include <stdint.h>
#include "backend/backend.h"

#ifndef MINEC_CLIENT_DYNAMIC_RENDERER_EXECUTABLE
#include <mutex.h>

#include "components.h"

#define RENDERER_ACCESS_PUBLIC_STATE(changes) {mutex_lock(&RENDERER.public.state.mutex); changes mutex_unlock(&RENDERER.public.state.mutex);}
#define RENDERER_ACCESS_ACTION_STATE(changes) {mutex_lock(&RENDERER.public.action.mutex); changes mutex_unlock(&RENDERER.public.action.mutex);}

#define RENDERER_MIN_FOV 70
#define RENDERER_MAX_FOV 150
#define RENDERER_MIN_RENDER_DISTANCE 2
#define RENDERER_MAX_RENDER_DISTANCE 32

#endif

enum _renderer_action_type
{
	_RENDERER_ACTION_SET_BACKEND_INDEX,
	_RENDERER_ACTION_SET_BACKEND_DEVICE_INDEX,
	_RENDERER_ACTION_SET_FPS,
	_RENDERER_ACTION_SET_VSYNC,
	_RENDERER_ACTION_SET_TRIPLE_BUFFERING,
	_RENDERER_ACTION_SET_MAX_MIPMAP_LEVEL_COUNT,
	_RENDERER_ACTION_SET_FOV,
	_RENDERER_ACTION_SET_RENDER_DISTANCE,
	_RENDERER_ACTION_RELOAD_ASSETS,
	_RENDERER_ACTION_RESTART,
};

struct renderer_action
{
	uint32_t type;

	union
	{
		uint32_t backend_index;
		uint32_t backend_device_index;
		uint32_t fps;
		bool vsync;
		bool triple_buffering;
		uint32_t max_mipmap_level_count;
		uint32_t fov;
		uint32_t render_distance;
		bool order_create_new_destroy_old;
	} parameters;
};

#define RENDERER_ACTION_SET_BACKEND_INDEX(backend_index)								&(struct renderer_action){.type = _RENDERER_ACTION_SET_BACKEND_INDEX,					.parameters.bakend_index = backend_index}
#define RENDERER_ACTION_SET_BACKEND_DEVICE_INDEX(backend_device_index)					&(struct renderer_action){.type = _RENDERER_ACTION_SET_BACKEND_DEVICE_INDEX,			.parameters.backend_device_index = backend_device_index}
#define RENDERER_ACTION_SET_FPS(fps)													&(struct renderer_action){.type = _RENDERER_ACTION_SET_FPS,								.parameters.fps = fps}
#define RENDERER_ACTION_SET_VSYNC(vsync)												&(struct renderer_action){.type = _RENDERER_ACTION_SET_VSYNC,							.parameters.vsync = vsync}
#define RENDERER_ACTION_SET_TRIPLE_BUFFERING(triple_buffering)							&(struct renderer_action){.type = _RENDERER_ACTION_SET_TRIPLE_BUFFERING,				.parameters.triple_buffering = triple_buffering}
#define RENDERER_ACTION_SET_MAX_MIPMAP_LEVEL_COUNT(max_mipmap_level_count)				&(struct renderer_action){.type = _RENDERER_ACTION_SET_MAX_MIPMAP_LEVEL_COUNT,			.parameters.max_mipmap_level_count = max_mipmap_level_count}
#define RENDERER_ACTION_SET_FOV(fov)													&(struct renderer_action){.type = _RENDERER_ACTION_SET_FOV,								.parameters.fov = fov}
#define RENDERER_ACTION_SET_RENDER_DISTANCE(render_distance)							&(struct renderer_action){.type = _RENDERER_ACTION_SET_RENDER_DISTANCE,					.parameters.render_distance = render_distance}
#define RENDERER_ACTION_RELOAD_ASSETS													&(struct renderer_action){.type = _RENDERER_ACTION_RELOAD_ASSETS}
#define RENDERER_ACTION_RESTART															&(struct renderer_action){.type = _RENDERER_ACTION_RESTART}

struct renderer_settings
{
	uint32_t backend_index;
	uint32_t backend_device_index;
	uint32_t fps;
	bool vsync;
	bool triple_buffering;
	uint32_t max_mipmap_level_count;
	uint32_t fov;
	uint32_t render_distance;
};

struct renderer_info_state
{
	struct renderer_backend_infos backend_infos;
	struct renderer_backend_device_infos backend_device_infos;
};

#if MINEC_CLIENT_DYNAMIC_RENDERER_LIBRARY

#define minec_client_log_info client->renderer.client_renderer_api._log_info
#define minec_client_log_error client->renderer.client_renderer_api._log_error

#ifdef MINEC_CLIENT_DEBUG_LOG

#define minec_client_log_debug client->renderer.client_renderer_api._log_debug
#define _minec_client_log_debug_l client->renderer.client_renderer_api._log_debug_l

#endif

#define asset_loader_get_asset client->renderer.client_renderer_api._asset_loader_get_asset
#define asset_loader_release_asset client->renderer.client_renderer_api._asset_loader_release_asset

#define RENDERER (*client->renderer._renderer)
#elif !defined(MINEC_CLIENT_DYNAMIC_RENDERER_EXECUTABLE)
#define RENDERER client->renderer
#endif

#if defined(MINEC_CLIENT_DYNAMIC_RENDERER_EXECUTABLE) || defined(MINEC_CLIENT_DYNAMIC_RENDERER_LIBRARY)

struct renderer
{
	void* library_handle;

	struct renderer_client_api
	{
		uint32_t(*_create)(struct minec_client* client, struct renderer_settings* settings);
		void(*_destroy)(struct minec_client* client);
		bool(*_did_crash)(struct minec_client* client);

		bool(*_get_info_state)(struct minec_client* client, struct renderer_info_state* info_state);
		bool(*_get_settings_state)(struct minec_client* client, struct renderer_settings* settings);
		void(*_action)(struct minec_client* client, struct renderer_action* action);

	} renderer_client_api;

	struct client_renderer_api
	{
		void (*_log_info)(struct minec_client* client, uint8_t* message, ...);
		void (*_log_error)(struct minec_client* client, uint8_t* message, ...);
		void (*_log_debug)(struct minec_client* client, uint8_t* message, ...);
		void (*_log_debug_l)(struct minec_client* client, uint8_t* function, uint8_t* file, uint32_t line, uint8_t* message, ...);

		void*(*_asset_loader_get_asset)(struct minec_client* client, uint8_t* name, size_t* size);
		void (*_asset_loader_release_asset)(struct minec_client* client);
	} client_renderer_api;

	struct _renderer* _renderer;
};

#endif

#ifndef MINEC_CLIENT_DYNAMIC_RENDERER_EXECUTABLE

struct renderer_action_state
{
	mutex_t mutex;

	struct { uint32_t value; bool change; } backend_index;
	struct { uint32_t value; bool change; } backend_device_index;
	struct { uint32_t value; bool change; } fps;
	struct { bool value; bool change; } vsync;
	struct { bool value; bool change; } use_triple_buffering;
	struct { uint32_t value; bool change; } max_mipmap_level_count;
	struct { uint32_t value; bool change; } fov;
	struct { uint32_t value; bool change; } render_distance;

	bool reload_assets;
	bool restart;
};

struct renderer_public_state
{
	mutex_t mutex;

	bool info_changed;
	struct renderer_info_state info;

	bool settings_changed;
	struct renderer_settings settings;
};

#if defined(MINEC_CLIENT_DYNAMIC_RENDERER_LIBRARY)
struct _renderer
#else
struct renderer
#endif
{
	void* thread_handle;

	struct renderer_settings settings;
	struct renderer_backend_device_infos backend_device_infos;
	struct renderer_components components;

	struct
	{
		float last_frame_time;

		uint32_t width;
		uint32_t height;
	} frame_info;

	struct
	{
		struct renderer_action_state action;
		struct renderer_public_state state;

		atomic_bool active;
		atomic_bool created;
	} public;
};
#endif

struct minec_client;

uint32_t renderer_create(struct minec_client* client, struct renderer_settings* settings);
void renderer_destroy(struct minec_client* client);
bool renderer_did_crash(struct minec_client* client);

bool renderer_get_info_state(struct minec_client* client, struct renderer_info_state* info_state);
bool renderer_get_settings_state(struct minec_client* client, struct renderer_settings* settings_state);
void renderer_action(struct minec_client* client, struct renderer_action* action);

#ifdef MINEC_CLIENT_DYNAMIC_RENDERER_EXECUTABLE
uint32_t renderer_reload(struct minec_client* client);
#else
void renderer_reset_action_state(struct minec_client* client);
#endif

#endif