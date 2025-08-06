#pragma once

#ifndef MINEC_CLIENT_RENDERER_RENDERER_H
#define MINEC_CLIENT_RENDERER_RENDERER_H

#define RENDRER_MAX_BACKEND_COUNT 8
#define RENDERER_MAX_BACKEND_DEVICE_COUNT 8

#include <stdint.h>
#include <pixelchar/pixelchar.h>
#include <mutex.h>

#include "frontend.h"
#include "backend.h"

#ifdef MINEC_CLIENT_RENDERER_BACKEND_OPENGL
#include "backend/opengl/backend_opengl.h"
#endif

#define RENDERER_ACCESS_INFO_STATE(changes) {mutex_lock(&RENDERER.public.info.mutex); changes mutex_unlock(&RENDERER.public.info.mutex);}
#define RENDERER_ACCESS_REQUEST_STATE(changes) {mutex_lock(&RENDERER.public.request.mutex); changes mutex_unlock(&RENDERER.public.request.mutex);}

#define RENDERER_PIXELCHAR_RENDERER_QUEUE_LENGTH 1024

struct renderer_backend_info
{
	uint8_t name[64];
};

struct renderer_backend_device_info
{
	uint8_t name[64];
	uint8_t version[64];

	bool usable;
	bool disable_vsync_support;
};

struct renderer_backend_infos
{
	struct renderer_backend_info infos[RENDRER_MAX_BACKEND_COUNT];
	uint32_t count;
};

struct renderer_backend_device_infos
{
	struct renderer_backend_device_info infos[RENDERER_MAX_BACKEND_DEVICE_COUNT];
	uint32_t count;
};

struct renderer_backend_info_state
{
	struct renderer_backend_infos backend_infos;
	struct renderer_backend_device_infos backend_device_infos;
};

struct renderer_backend_settings
{
	uint32_t backend_index;
	uint32_t backend_device_index;
	uint32_t fps;
	bool vsync;
	uint32_t max_mipmap_level_count;
};
struct renderer_frontend_settings
{
	uint32_t fov;
	uint32_t gui_scale;
	uint32_t render_distance;
};
struct renderer_other_settings
{
	bool order_create_new_destroy_old;
};

struct renderer_settings
{
	struct renderer_backend_settings backend;
	struct renderer_frontend_settings frontend;
	struct renderer_other_settings other;
};

struct renderer_info_state
{
	struct renderer_backend_info_state backend;
	struct renderer_settings settings;
};

#if MINEC_CLIENT_DYNAMIC_RENDERER_LIBRARY

#define minec_client_log_info client->renderer.client_renderer_api._log_info
#define minec_client_log_error client->renderer.client_renderer_api._log_error

#ifdef MINEC_CLIENT_DEBUG_LOG

#define minec_client_log_debug client->renderer.client_renderer_api._log_debug
#define _minec_client_log_debug_l client->renderer.client_renderer_api._log_debug_l

#endif

#define application_window_get_dimensions client->renderer.client_renderer_api._application_window_get_dimensions

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

		void(*_set_order_create_new_destroy_old)(struct minec_client* client, bool order);
		void(*_reload_resources)(struct minec_client* client);
		void(*_switch_backend)(struct minec_client* client, uint32_t backend_index);
		void(*_switch_backend_device)(struct minec_client* client, uint32_t backend_device_index);
		void(*_set_vsync)(struct minec_client* client, bool vsync);
		void(*_set_fps)(struct minec_client* client, uint32_t fps);
		void(*_set_gui_scale)(struct minec_client* client, uint32_t scale);
		void(*_set_fov)(struct minec_client* client, uint32_t fov);
		void(*_set_render_distance)(struct minec_client* client, uint32_t render_distance);
		void(*_set_max_mipmap_level_count)(struct minec_client* client, uint32_t max_mipmap_level_count);
	} renderer_client_api;

	struct client_renderer_api
	{
		void (*_log_info)(struct minec_client* client, uint8_t* message, ...);
		void (*_log_error)(struct minec_client* client, uint8_t* message, ...);
		void (*_log_debug)(struct minec_client* client, uint8_t* message, ...);
		void (*_log_debug_l)(struct minec_client* client, uint8_t* function, uint8_t* file, uint32_t line, uint8_t* message, ...);

		void (*_application_window_get_dimensions)(struct minec_client* client, uint32_t* width, uint32_t* height);
	} client_renderer_api;

	struct _renderer* _renderer;
};

#endif

#ifndef MINEC_CLIENT_DYNAMIC_RENDERER_EXECUTABLE

struct renderer_backend
{
	struct renderer_backend_device_infos device_infos;
	bool device_infos_changed;

	struct renderer_backend_settings settings;

	union renderer_backend_internal_state
	{
#ifdef MINEC_CLIENT_RENDERER_BACKEND_OPENGL
		struct renderer_backend_opengl opengl;
#endif
	} state;

	uint32_t pixelchar_slots[2];
	uint32_t pixelchar_slot_index;
};

struct renderer_frontend
{
	struct renderer_frontend_settings settings;

	PixelcharRenderer pixelchar_renderer;

	struct
	{
		float last_frame_time;

		uint32_t width;
		uint32_t height;
	} frame_info;
};

#if defined(MINEC_CLIENT_DYNAMIC_RENDERER_LIBRARY)
struct _renderer
#else
struct renderer
#endif
{
	void* thread_handle;

	bool crashing;

	struct renderer_frontend frontend;
	struct renderer_backend backend;
	bool backend_exists;

	struct renderer_other_settings settings;

	struct
	{
		struct
		{
			mutex_t mutex;
			struct renderer_settings settings;
			bool reload_resources;
			bool reload_resources_detected;
			bool restart;
		} request;

		struct
		{
			mutex_t mutex;
			bool changed;
			struct renderer_info_state state;
		} info;

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

#ifdef MINEC_CLIENT_DYNAMIC_RENDERER_EXECUTABLE
uint32_t renderer_reload(struct minec_client* client);
#endif

void renderer_set_order_create_new_destroy_old(struct minec_client* client, bool order);
void renderer_switch_backend(struct minec_client* client, uint32_t backend_index);
void renderer_switch_backend_device(struct minec_client* client, uint32_t backend_device_index);
void renderer_reload_resources(struct minec_client* client);
void renderer_set_vsync(struct minec_client* client, bool vsync);
void renderer_set_fps(struct minec_client* client, uint32_t fps);
void renderer_set_gui_scale(struct minec_client* client, uint32_t scale);
void renderer_set_fov(struct minec_client* client, uint32_t fov);
void renderer_set_render_distance(struct minec_client* client, uint32_t render_distance);
void renderer_set_max_mipmap_level_count(struct minec_client* client, uint32_t max_mipmap_level_count);

#endif