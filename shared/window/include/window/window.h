#pragma once

#ifndef WINDOW_H
#define WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

enum window_key {

	WINDOW_KEY_1,
	WINDOW_KEY_2,
	WINDOW_KEY_3,
	WINDOW_KEY_4,
	WINDOW_KEY_5,
	WINDOW_KEY_6,
	WINDOW_KEY_7,
	WINDOW_KEY_8,
	WINDOW_KEY_9,
	WINDOW_KEY_0,

	WINDOW_KEY_A,
	WINDOW_KEY_B,
	WINDOW_KEY_C,
	WINDOW_KEY_D,
	WINDOW_KEY_E,
	WINDOW_KEY_F,
	WINDOW_KEY_G,
	WINDOW_KEY_H,
	WINDOW_KEY_I,
	WINDOW_KEY_J,
	WINDOW_KEY_K,
	WINDOW_KEY_L,
	WINDOW_KEY_M,
	WINDOW_KEY_N,
	WINDOW_KEY_O,
	WINDOW_KEY_P,
	WINDOW_KEY_Q,
	WINDOW_KEY_R,
	WINDOW_KEY_S,
	WINDOW_KEY_T,
	WINDOW_KEY_U,
	WINDOW_KEY_V,
	WINDOW_KEY_W,
	WINDOW_KEY_X,
	WINDOW_KEY_Y,
	WINDOW_KEY_Z,

	WINDOW_KEY_PLUS,
	WINDOW_KEY_MINUS,

	WINDOW_KEY_F1,
	WINDOW_KEY_F2,
	WINDOW_KEY_F3,
	WINDOW_KEY_F4,
	WINDOW_KEY_F5,
	WINDOW_KEY_F6,
	WINDOW_KEY_F7,
	WINDOW_KEY_F8,
	WINDOW_KEY_F9,
	WINDOW_KEY_F10,
	WINDOW_KEY_F11,
	WINDOW_KEY_F12,

	WINDOW_KEY_SHIFT_L,
	WINDOW_KEY_SHIFT_R,
	WINDOW_KEY_CTRL_L,
	WINDOW_KEY_CTRL_R,
	WINDOW_KEY_ALT_L,
	WINDOW_KEY_ALT_R,

	WINDOW_KEY_SPACE,
	WINDOW_KEY_BACKSPACE,
	WINDOW_KEY_TAB,
	WINDOW_KEY_ENTER,
	WINDOW_KEY_ESCAPE,

	WINDOW_KEY_ARROW_UP,
	WINDOW_KEY_ARROW_DOWN,
	WINDOW_KEY_ARROW_LEFT,
	WINDOW_KEY_ARROW_RIGHT,

	WINDOW_KEY_MOUSE_LEFT,
	WINDOW_KEY_MOUSE_MIDDLE,
	WINDOW_KEY_MOUSE_RIGHT,

	WINDOW_KEY_TOTAL_COUNT,
};

enum window_event_type {
	WINDOW_EVENT_DESTROY,
	WINDOW_EVENT_FOCUS,
	WINDOW_EVENT_UNFOCUS,
	WINDOW_EVENT_MOUSE_SCROLL,
	WINDOW_EVENT_CHARACTER,
	WINDOW_EVENT_KEY_UP,
	WINDOW_EVENT_KEY_DOWN,
	WINDOW_EVENT_MOVE_SIZE,
};

struct window_event {
	uint32_t type;
	union {
		struct {
			int32_t scroll_steps;
		} mouse_scroll;
		struct {
			uint32_t code_point;
		} character;
		struct {
			uint32_t key;
		} key_down;
		struct {
			uint32_t key;
		} key_up;
		struct {
			uint32_t width;
			uint32_t height;
			uint32_t position_x;
			uint32_t position_y;
		} move_size;
	} info;
};

bool window_init_context(void* context);
void window_deinit_context();
void* window_get_context();

void* window_create(int32_t posx, int32_t posy, uint32_t width, uint32_t height, uint8_t* name, bool visible);
bool window_destroy(void* window);

bool window_set_icon(void* window, uint32_t* icon_rgba_pixel_data, uint32_t icon_width, uint32_t icon_height);
void window_get_dimensions(void* window, uint32_t* width, uint32_t* height, int32_t* screen_position_x, int32_t* screen_position_y);
bool window_is_selected(void* window);
void window_get_mouse_cursor_position(void* window, int32_t* position_x, int32_t* position_y);
void window_set_mouse_cursor_position(void* window, int32_t x, int32_t y);
struct window_event* window_next_event(void* window);

#ifdef WINDOW_IMPLEMENTATION_WINDOWS

#include <Windows.h>
HWND window_windows_get_hwnd(void* window);

#endif

#ifdef WINDOW_IMPLEMENTATION_X11


#endif

//vulkan
#ifdef WINDOW_GRAPHICS_VULKAN

#include <vulkan/vulkan.h>
bool window_vulkan_load();
void window_vulkan_unload();

PFN_vkGetInstanceProcAddr window_get_vkGetInstanceProcAddr();
VkResult window_vkCreateSurfaceKHR(void* window, VkInstance instance, VkSurfaceKHR* surface);
uint8_t* window_get_VK_KHR_PLATFORM_SURFACE_EXTENSION_NAME();

#endif

//opengl
#ifdef WINDOW_GRAPHICS_OPENGL
#include <GL/glcorearb.h>

bool window_opengl_load();
void window_opengl_unload();

bool window_glCreateContext(void* window, int32_t version_major, int32_t version_minor, void* share_window, bool* glSwapIntervalEXT_support);
bool window_glDestroyContext(void* window);

bool window_glMakeCurrent(void* window);
bool window_glSwapIntervalEXT(int interval);
void (*window_glGetProcAddress(uint8_t* name)) (void);
bool window_glSwapBuffers(void* window);

#endif

#ifdef __cplusplus
}
#endif

#endif
