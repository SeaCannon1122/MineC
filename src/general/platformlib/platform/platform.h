#pragma once

#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>

enum platform_keys {

	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_0,

	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,

	KEY_PLUS,
	KEY_MINUS,

	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,

	KEY_SHIFT_L,
	KEY_SHIFT_R,
	KEY_CTRL_L,
	KEY_CTRL_R,
	KEY_ALT_L,
	KEY_ALT_R,

	KEY_SPACE,
	KEY_BACKSPACE,
	KEY_TAB,
	KEY_ENTER,
	KEY_ESCAPE,

	KEY_ARROW_UP,
	KEY_ARROW_DOWN,
	KEY_ARROW_LEFT,
	KEY_ARROW_RIGHT,

	KEY_MOUSE_LEFT,
	KEY_MOUSE_MIDDLE,
	KEY_MOUSE_RIGHT,

	KEY_TOTAL_COUNT,
};

enum window_event_type {
	WINDOW_EVENT_NULL,
	WINDOW_EVENT_DESTROY,
	WINDOW_EVENT_FOCUS,
	WINDOW_EVENT_UNFOCUS,
	WINDOW_EVENT_MOUSE_SCROLL,
	WINDOW_EVENT_CHAR,
	WINDOW_EVENT_KEY_UP,
	WINDOW_EVENT_KEY_DOWN,
	WINDOW_EVENT_MOVE_SIZE,
};

struct point2d_int {
	int32_t x;
	int32_t y;
};

struct window_event {
	uint32_t type;
	union {
		struct {
			int32_t scroll_steps;
		} event_mouse_scroll;
		struct {
			uint32_t unicode;
		} event_char;
		struct {
			uint32_t key;
		} event_key_down;
		struct {
			uint32_t key;
		} event_key_up;
		struct {
			uint32_t width;
			uint32_t height;
			uint32_t position_x;
			uint32_t position_y;
		} event_move_size;
	} info;
};

#define MAX_WINDOW_COUNT 4

#define WINDOW_CREATION_FAILED -1

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
#include <vulkan/vulkan.h>
#define PLATFORM_VK_SURFACE_EXTENSION VK_KHR_XLIB_SURFACE_EXTENSION_NAME

#elif defined(__APPLE__)

#define DEBUG_BREAK __builtin_trap()
#define RESTRICT restrict

#endif

void* dynamic_library_load(uint8_t* src);
void (*dynamic_library_get_function(void* library_handle, uint8_t* function_name)) (void);
void dynamic_library_unload(void* library_handle);

uint32_t directory_exists(uint8_t* path);
uint32_t create_directory(uint8_t* path);
uint32_t get_cwd(uint8_t* buffer, size_t buffer_size);

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

uint32_t get_screen_width();
uint32_t get_screen_height();

//window functions

uint32_t window_create(uint32_t posx, uint32_t posy, uint32_t width, uint32_t height, uint8_t* name, uint32_t visible);

uint32_t window_get_width(uint32_t window);
uint32_t window_get_height(uint32_t window);

uint32_t window_get_x_position(uint32_t window);
uint32_t window_get_y_position(uint32_t window);

int32_t window_is_selected(uint32_t window);

void window_destroy(uint32_t window);

struct point2d_int window_get_mouse_cursor_position(uint32_t window);

void window_set_mouse_cursor_position(uint32_t window, int32_t x, int32_t y);

uint32_t window_process_next_event(uint32_t window, struct window_event* event);

VkResult create_vulkan_surface(VkInstance instance, uint32_t window, VkSurfaceKHR* surface);

VkResult destroy_vulkan_surface(VkInstance instance, VkSurfaceKHR surface);


#endif