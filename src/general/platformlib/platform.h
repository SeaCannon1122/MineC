#ifndef PLATFORM_H
#define PLATFORM_H



#if defined(_WIN32)

#define DEBUG_BREAK() __debugbreak()

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#define PLATFORM_VK_SURFACE_EXTENSION VK_KHR_WIN32_SURFACE_EXTENSION_NAME

#elif defined(__linux__)
#include <signal.h>
#define DEBUG_BREAK() raise(SIGTRAP)

#define VK_USE_PLATFORM_XLIB_KHR
#include <vulkan/vulkan.h>
#define PLATFORM_VK_SURFACE_EXTENSION VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#endif



#ifndef MAX_WINDOW_COUNT
#define MAX_WINDOW_COUNT 5
#endif // !MAX_WINDOW_COUNT

#ifndef MAX_CALLBACK_FUNCTIONS
#define MAX_CALLBACK_FUNCTIONS 32
#endif // !1

#define WINDOW_CREATION_FAILED -1
#define KEYBOARD_BUFFER_PARSER_CREATION_FAILED -1

struct point2d_int {
	int x;
	int y;
};

void* dynamic_library_load(char* src);
void (*dynamic_library_get_function(void* library_handle, char* function_name)) (void);
void dynamic_library_unload(void* library_handle);

void platform_init();
void platform_exit();

//general
void show_console_window();
void hide_console_window();

void set_console_cursor_position(int x, int y);

void sleep_for_ms(unsigned int time_in_milliseconds);

double get_time();

void* create_thread(void (address) (void*), void* args);

void join_thread(void* thread_handle);

char get_key_state(int key);

//window functions

int window_create(int posx, int posy, int width, int height, unsigned char* name);

int window_get_width(int window);

int window_get_height(int window);

int window_is_selected(int window);

int window_is_active(int window);

void window_destroy(int window);

void window_draw(int window, unsigned int* buffer, int width, int height, int scalar);

struct point2d_int window_get_mouse_cursor_position(int window);

void window_set_mouse_cursor_position(int window, int x, int y);

int window_get_last_mouse_scrolls(int window);

void window_clear_mouse_scrolls(int window);

int window_add_char_callback(int window, void (*callback) (int, int));

void window_remove_char_callback(int window, int char_callback_id);

VkResult create_vulkan_surface(VkInstance instance, int window, VkSurfaceKHR* surface);

//#elif defined(__linux__)
//#ifdef VK_USE_PLATFORM_XLIB_KHR
//#include <vulkan/vulkan_xlib.h>
//#define PLATFORM_VK_SURFACE_EXTENSION VK_KHR_XLIB_SURFACE_EXTENSION_NAME
//#endif
//#endif

//keysymbol Mapping

#if defined(_WIN32)

#define RESTRICT __restrict

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

#define RESTRICT restrict

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
