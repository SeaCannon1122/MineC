#pragma once

#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>

enum window_event_type {
	WINDOW_EVENT_NULL,
	WINDOW_EVENT_DESTROY,
	WINDOW_EVENT_MOVE,
	WINDOW_EVENT_FOCUS,
	WINDOW_EVENT_UNFOCUS,
	WINDOW_EVENT_MOUSE_SCROLL,
	WINDOW_EVENT_CHAR,
	WINDOW_EVENT_SIZE,
};

#if defined(_WIN32)

#define DEBUG_BREAK() __debugbreak()
#define RESTRICT __restrict
#define DLL_EXPORT __declspec(dllexport)

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#define PLATFORM_VK_SURFACE_EXTENSION VK_KHR_WIN32_SURFACE_EXTENSION_NAME

#elif defined(__linux__)
#include <signal.h>
#define DEBUG_BREAK() raise(SIGTRAP)
#define RESTRICT restrict
#define DLL_EXPORT __attribute__((visibility("default")))

#define VK_USE_PLATFORM_XLIB_KHR
#define VK_USE_PLATFORM_XLIB_KHR
#include <vulkan/vulkan.h>
#define PLATFORM_VK_SURFACE_EXTENSION VK_KHR_XLIB_SURFACE_EXTENSION_NAME

#elif defined(__APPLE__)

#define DEBUG_BREAK __builtin_trap()
#define RESTRICT restrict

#endif



#ifndef MAX_WINDOW_COUNT
#define MAX_WINDOW_COUNT 4
#endif // !MAX_WINDOW_COUNT

#ifndef MAX_CHAR_CALLBACK_FUNCTIONS
#define MAX_CHAR_CALLBACK_FUNCTIONS 8
#endif // !1

#define WINDOW_CREATION_FAILED -1
#define KEYBOARD_BUFFER_PARSER_CREATION_FAILED -1

struct point2d_int {
	int32_t x;
	int32_t y;
};

struct window_event {
	uint32_t type;
	uint32_t window;
	union {
		struct {
			uint32_t scroll_steps;
		} window_event_mouse_scroll;
		struct {
			uint32_t utf8_char;
			uint32_t utf16_char;
			uint32_t unicode;
		} window_event_char;
		struct {
			uint32_t key;
		} window_event_key_down;
		struct {
			uint32_t key;
		} window_event_key_up;
		struct {
			uint32_t width;
			uint32_t height;
		} window_event_size;
		struct {
			uint32_t x_position;
			uint32_t y_position;
		} window_event_move;
	} info;
};

void* dynamic_library_load(uint8_t* src);
void (*dynamic_library_get_function(void* library_handle, uint8_t* function_name)) (void);
void dynamic_library_unload(void* library_handle);

void platform_init();
void platform_exit();

//general
void show_console_window();
void hide_console_window();

void set_console_cursor_position(int32_t x, int32_t y);

void sleep_for_ms(uint32_t time_in_milliseconds);

double get_time();

void* create_thread(void (address) (void*), void* args);

void join_thread(void* thread_handle);

int8_t get_key_state(int32_t key);

//window functions

uint32_t window_create(uint32_t posx, uint32_t posy, uint32_t width, uint32_t height, uint8_t* name);

uint32_t window_get_width(uint32_t window);
uint32_t window_get_height(uint32_t window);

uint32_t window_get_x_position(uint32_t window);
uint32_t window_get_y_position(uint32_t window);

int32_t window_is_selected(uint32_t window);

int32_t window_is_active(uint32_t window);

void window_destroy(uint32_t window);

void window_draw(uint32_t window, uint8_t* buffer, int32_t width, int32_t height, int32_t scalar);

struct point2d_int window_get_mouse_cursor_position(uint32_t window);

void window_set_mouse_cursor_position(uint32_t window, int32_t x, int32_t y);

uint32_t window_process_next_event(struct window_event* event);

VkResult create_vulkan_surface(VkInstance instance, uint32_t window, VkSurfaceKHR* surface);

//keysymbol Mapping

#if defined(_WIN32)

#include <windows.h>

#define KEY_SPACE VK_SPACE
#define KEY_SHIFT_L VK_SHIFT
#define KEY_SHIFT_R VK_RSHIFT
#define KEY_CONTROL_L VK_CONTROL
#define KEY_CONTROL_R VK_RCONTROL
#define KEY_ALT_L VK_LMENU
#define KEY_ALT_R VK_RMENU
#define KEY_ESCAPE VK_ESCAPE
#define KEY_BACKSPACE VK_BACK
#define KEY_TAB VK_TAB
#define KEY_ENTER VK_RETURN
#define KEY_CAPS_LOCK VK_CAPITAL
#define KEY_MINUS VK_OEM_MINUS
#define KEY_PLUS VK_OEM_PLUS
#define KEY_ARROW_LEFT VK_LEFT
#define KEY_ARROW_RIGHT VK_RIGHT
#define KEY_ARROW_UP VK_UP
#define KEY_ARROW_DOWN VK_DOWN
#define KEY_MOUSE_LEFT VK_LBUTTON
#define KEY_MOUSE_MIDDLE VK_MBUTTON
#define KEY_MOUSE_RIGHT VK_RBUTTON

#elif defined(__linux__)

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>

#define KEY_SPACE XK_space
#define KEY_SHIFT_L XK_Shift_L
#define KEY_SHIFT_R XK_Shift_R
#define KEY_CONTROL_L XK_Control_L
#define KEY_CONTROL_R XK_Control_R
#define KEY_ESCAPE XK_Escape
#define KEY_BACKSPACE XK_BackSpace
#define KEY_ALT_L XK_Alt_L
#define KEY_ALT_R XK_Alt_R
#define KEY_TAB XK_Tab
#define KEY_ENTER XK_Return
#define KEY_CAPS_LOCK XK_Caps_Lock
#define KEY_MINUS XK_minus
#define KEY_PLUS XK_plus
#define KEY_ARROW_LEFT XK_Left
#define KEY_ARROW_RIGHT XK_Right
#define KEY_ARROW_UP XK_Up
#define KEY_ARROW_DOWN XK_Down
#define KEY_MOUSE_LEFT 0x1234
#define KEY_MOUSE_MIDDLE 0x1235
#define KEY_MOUSE_RIGHT 0x1236

#endif

#endif // PLATFORM_H
