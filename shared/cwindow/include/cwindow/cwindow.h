#pragma once

#ifndef CWINDOW_H
#define CWINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum cwindow_key
{
	CWINDOW_KEY_UNKNOWN = 0,

	CWINDOW_KEY_1,
	CWINDOW_KEY_2,
	CWINDOW_KEY_3,
	CWINDOW_KEY_4,
	CWINDOW_KEY_5,
	CWINDOW_KEY_6,
	CWINDOW_KEY_7,
	CWINDOW_KEY_8,
	CWINDOW_KEY_9,
	CWINDOW_KEY_0,

	CWINDOW_KEY_A,
	CWINDOW_KEY_B,
	CWINDOW_KEY_C,
	CWINDOW_KEY_D,
	CWINDOW_KEY_E,
	CWINDOW_KEY_F,
	CWINDOW_KEY_G,
	CWINDOW_KEY_H,
	CWINDOW_KEY_I,
	CWINDOW_KEY_J,
	CWINDOW_KEY_K,
	CWINDOW_KEY_L,
	CWINDOW_KEY_M,
	CWINDOW_KEY_N,
	CWINDOW_KEY_O,
	CWINDOW_KEY_P,
	CWINDOW_KEY_Q,
	CWINDOW_KEY_R,
	CWINDOW_KEY_S,
	CWINDOW_KEY_T,
	CWINDOW_KEY_U,
	CWINDOW_KEY_V,
	CWINDOW_KEY_W,
	CWINDOW_KEY_X,
	CWINDOW_KEY_Y,
	CWINDOW_KEY_Z,

	CWINDOW_KEY_PLUS,
	CWINDOW_KEY_MINUS,

	CWINDOW_KEY_F1,
	CWINDOW_KEY_F2,
	CWINDOW_KEY_F3,
	CWINDOW_KEY_F4,
	CWINDOW_KEY_F5,
	CWINDOW_KEY_F6,
	CWINDOW_KEY_F7,
	CWINDOW_KEY_F8,
	CWINDOW_KEY_F9,
	CWINDOW_KEY_F10,
	CWINDOW_KEY_F11,
	CWINDOW_KEY_F12,

	CWINDOW_KEY_SHIFT_L,
	CWINDOW_KEY_SHIFT_R,
	CWINDOW_KEY_CTRL_L,
	CWINDOW_KEY_CTRL_R,
	CWINDOW_KEY_ALT_L,
	CWINDOW_KEY_ALT_R,

	CWINDOW_KEY_SPACE,
	CWINDOW_KEY_BACKSPACE,
	CWINDOW_KEY_TAB,
	CWINDOW_KEY_ENTER,
	CWINDOW_KEY_ESCAPE,

	CWINDOW_KEY_ARROW_UP,
	CWINDOW_KEY_ARROW_DOWN,
	CWINDOW_KEY_ARROW_LEFT,
	CWINDOW_KEY_ARROW_RIGHT,

	CWINDOW_KEY_MOUSE_LEFT,
	CWINDOW_KEY_MOUSE_MIDDLE,
	CWINDOW_KEY_MOUSE_RIGHT,

	CWINDOW_KEY_s_COUNT
} cwindow_key;

typedef enum cwindow_event_type
{
	CWINDOW_EVENT_UNKNOWN = 0,

	CWINDOW_EVENT_DESTROY,
	CWINDOW_EVENT_FOCUS,
	CWINDOW_EVENT_UNFOCUS,
	CWINDOW_EVENT_MOUSE_SCROLL,
	CWINDOW_EVENT_CHARACTER,
	CWINDOW_EVENT_KEY_UP,
	CWINDOW_EVENT_KEY_DOWN,
	CWINDOW_EVENT_SIZE,
	CWINDOW_EVENT_MOVE,
} cwindow_event_type;

typedef struct cwindow_context cwindow_context;
typedef struct cwindow cwindow;

typedef struct cwindow_event
{
	cwindow_event_type type;
	union
	{
		struct
		{
			int32_t scroll_steps;
		} mouse_scroll;
		struct
		{
			uint32_t character;
		} character;
		struct
		{
			cwindow_key key;
		} key_down_up;
		struct
		{
			uint32_t width;
			uint32_t height;
		} size;
		struct
		{
			uint32_t posx;
			uint32_t posy;
		} move;
	} info;
} cwindow_event;

typedef void (*pfn_cwindow_event_callback)(cwindow* window, const cwindow_event* event, void* user_parameter);

cwindow_context* cwindow_context_create(const uint8_t* name);
void cwindow_context_destroy(cwindow_context* context);
void cwindow_context_get_display_dimensions(cwindow_context* context, uint32_t* width, uint32_t* height);

cwindow* cwindow_create(cwindow_context* context, int32_t posx, int32_t posy, uint32_t width, uint32_t height, const uint8_t* name, bool visible, pfn_cwindow_event_callback event_callback);
void cwindow_destroy(cwindow* window);

void cwindow_set_event_callback_user_parameter(cwindow* window, void* user_parameter);
bool cwindow_set_icon(cwindow* window, const uint32_t* icon_rgba_pixel_data, uint32_t icon_width, uint32_t icon_height);
void cwindow_get_dimensions(cwindow* window, uint32_t* width, uint32_t* height, int32_t* posx, int32_t* posy);
bool cwindow_is_selected(cwindow* window);
void cwindow_get_mouse_cursor_position(cwindow* window, int32_t* posx, int32_t* posy);
void cwindow_set_mouse_cursor_position(cwindow* window, int32_t posx, int32_t posy);
void cwindow_handle_events(cwindow* window);

#ifdef CWINDOW_IMPLEMENTATION_WINDOWS

#define CWINDOW_GRAPHICS_VULKAN
#define CWINDOW_GRAPHICS_OPENGL

#define VK_USE_PLATFORM_WIN32_KHR
#define CWINDOW_VK_KHR_PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME

#include <Windows.h>

HWND cwindow_impl_windows_get_hwnd(cwindow* window);

#endif

#ifdef CWINDOW_IMPLEMENTATION_X11

#define CWINDOW_GRAPHICS_VULKAN
#define CWINDOW_GRAPHICS_OPENGL

#define VK_USE_PLATFORM_XLIB_KHR
#define CWINDOW_VK_KHR_PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_XLIB_SURFACE_EXTENSION_NAME

#include <X11/Xlib.h>

Display* window_impl_x11_context_get_display(cwindow_context* context);
Window window_impl_x11_get_window(cwindow* window);

#endif

#ifdef CWINDOW_GRAPHICS_VULKAN

#include <vulkan/vulkan.h>

bool cwindow_context_graphics_vulkan_load(cwindow_context* context, PFN_vkGetInstanceProcAddr* gpa);
void cwindow_context_graphics_vulkan_unload(cwindow_context* context);

VkResult cwindow_vkCreateSurfaceKHR(cwindow* window, VkInstance instance, VkSurfaceKHR* surface);

#endif

#ifdef CWINDOW_GRAPHICS_OPENGL

#include <GL/glcorearb.h>

bool cwindow_context_graphics_opengl_load(cwindow_context* context);
void cwindow_context_graphics_opengl_unload(cwindow_context* context);

bool cwindow_glCreateContext(cwindow* window, int32_t version_major, int32_t version_minor, cwindow* share_window, bool* glSwapIntervalEXT_support);
void cwindow_glDestroyContext(cwindow* window);

bool cwindow_glMakeCurrent(cwindow_context* context, cwindow* window);
bool cwindow_glSwapIntervalEXT(cwindow* window, int interval);
void (*cwindow_glGetProcAddress(cwindow* window, const uint8_t* name)) (void);
bool cwindow_glSwapBuffers(cwindow* window);

#endif

#ifdef __cplusplus
}
#endif

#endif