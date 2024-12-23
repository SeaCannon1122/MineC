#include "platform.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/Xlocale.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

struct window_state {
	Window window;
	XIC ic;
	unsigned int* pixels;
	int32_t active;
	int32_t window_width;
	int32_t window_height;
	int32_t window_x_pos;
	int32_t window_y_pos;
};

struct window_state window_states[MAX_WINDOW_COUNT];


int32_t display_width;
int32_t display_height;

Display* display;
int32_t screen;
Atom wm_delete_window;
XIM xim;

int32_t running = 1;

char keyStates[256 * 256] = { 0 };
char mouseButtons[3] = { 0, 0, 0 };

int32_t move_event = 0;
int32_t move_window;
int32_t move_x;
int32_t move_y;

void* dynamic_library_load(uint8_t* src) {

	int32_t src_length = 0;
	for (; src[src_length] != 0; src_length++);

	char* src_platform = alloca(sizeof("lib") - 1 + src_length + sizeof(".so"));

	src_platform[0] = 'l';
	src_platform[1] = 'i';
	src_platform[2] = 'b';
	for (int32_t i = 3; i < src_length + 3; i++) src_platform[i] = src[i - 3];
	src_platform[src_length + 3] = '.';
	src_platform[src_length + 4] = 's';
	src_platform[src_length + 5] = 'o';
	src_platform[src_length + 6] = '\0';

	void* handle = dlopen(src_platform, RTLD_NOW);
	if (!handle) {
		printf("Error loading shared object: %s\n", dlerror());
		return NULL;
	}

	return handle;
}

void (*dynamic_library_get_function(void* library_handle, uint8_t* function_name)) (void) {
	return dlsym(library_handle, function_name);
}

void dynamic_library_unload(void* library_handle) {
	dlclose(library_handle);
}

void show_console_window() { return; }

void hide_console_window() { return; }

void set_console_cursor_position(int32_t x, int32_t y) {
	printf("\033[%d;%dH", y, x);
}

void sleep_for_ms(uint32_t time_in_milliseconds) {
	usleep(time_in_milliseconds * 1000);
}

double get_time() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double)tv.tv_sec * 1000. + (double)tv.tv_usec / 1000.;
}

void* create_thread(void (address)(void*), void* args) {
	pthread_t* thread = malloc(sizeof(pthread_t));
	pthread_create(thread, NULL, (void* (*)(void*))address, args);
	return thread;
}

void join_thread(void* thread_handle) {
	pthread_join(*(pthread_t*)thread_handle, NULL);
	free(thread_handle);
}

int8_t get_key_state(int32_t key) {

	char key_state = 0;

	if (key == KEY_MOUSE_LEFT || key == KEY_MOUSE_MIDDLE || key == KEY_MOUSE_RIGHT) {

		Window root = DefaultRootWindow(display);
		Window root_return, child_return;
		int32_t root_x, root_y, win_x, win_y;
		uint32_t mask_return;
		XQueryPointer(display, root, &root_return, &child_return, &root_x, &root_y, &win_x, &win_y, &mask_return);

		if (mask_return & (key == KEY_MOUSE_LEFT ? Button1Mask : (key == KEY_MOUSE_MIDDLE ? Button2Mask : Button3Mask))) key_state = 0b1;

		if (key_state != mouseButtons[(key == KEY_MOUSE_LEFT ? 0 : (key == KEY_MOUSE_MIDDLE ? 1 : 2))]) key_state |= 0b10;

		mouseButtons[(key == KEY_MOUSE_LEFT ? 0 : (key == KEY_MOUSE_MIDDLE ? 1 : 2))] = key_state & 0b1;

		return key_state;
	}

	char keys[32];
	XQueryKeymap(display, keys);

	KeySym keysym = (KeySym)key;
	KeyCode keycode = XKeysymToKeycode(display, keysym);

	int32_t byteIndex = keycode / 8;
	int32_t bitIndex = keycode % 8;

	if (keys[byteIndex] & (1 << bitIndex)) key_state = 0b1;
	if (key_state != keyStates[key]) key_state |= 0b10;
	keyStates[key] = key_state & 0b1;

	return key_state;
}

uint32_t get_screen_width() {
	return display_width;
}

uint32_t get_screen_height() {
	return display_height;
}

uint32_t window_create(uint32_t posx, uint32_t posy, uint32_t width, uint32_t height, uint8_t* name, uint32_t visible) {

	uint32_t next_free_window_index = 0;
	for (; next_free_window_index < MAX_WINDOW_COUNT; next_free_window_index++) if (window_states[next_free_window_index].window == 0) break;
	if (next_free_window_index == MAX_WINDOW_COUNT) return WINDOW_CREATION_FAILED;

	Window window = XCreateSimpleWindow(display, RootWindow(display, screen), posx, posy, width, height, 1, BlackPixel(display, screen), WhitePixel(display, screen));

	XIC ic = XCreateIC(xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, window, NULL);

	unsigned int* pixels = malloc(display_width * display_height * sizeof(unsigned int));

	XSelectInput(display, window, ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask | ButtonPressMask);
	XStoreName(display, window, name);
	XSetWMProtocols(display, window, &wm_delete_window, 1);
	if(visible) XMapWindow(display, window);

	window_states[next_free_window_index].window = window;
	window_states[next_free_window_index].ic = ic;
	window_states[next_free_window_index].pixels = pixels;
	window_states[next_free_window_index].window_width = width;
	window_states[next_free_window_index].window_height = height;
	window_states[next_free_window_index].window_x_pos = posx;
	window_states[next_free_window_index].window_y_pos = posy;
	window_states[next_free_window_index].active = 1;

	return next_free_window_index;
}

uint32_t window_get_width(uint32_t window) {
	return window_states[window].window_width;
}

uint32_t window_get_height(uint32_t window) {
	return window_states[window].window_height;
}

uint32_t window_get_x_position(uint32_t window) {
	return window_states[window].window_x_pos;
}

uint32_t window_get_y_position(uint32_t window) {
	return window_states[window].window_y_pos;
}

int32_t window_is_selected(uint32_t window) {
	Window focused_window;
	int32_t revert_to;

	XGetInputFocus(display, &focused_window, &revert_to);

	return focused_window == window_states[window].window;
}

int32_t window_is_active(uint32_t window) {
	return window_states[window].active;
}

void window_destroy(uint32_t window) {

	if (window_states[window].active) XDestroyWindow(display, window_states[window].window);
	window_states[window].window = 0;

	
}

struct point2d_int window_get_mouse_cursor_position(uint32_t window) {
	if (!window_states[window].active) return (struct point2d_int) { -1, -1 };
	Window root, child;
	int32_t root_x, root_y;
	int32_t win_x, win_y;
	uint32_t mask;

	XQueryPointer(display, window_states[window].window, &root, &child, &root_x, &root_y, &win_x, &win_y, &mask);

	struct point2d_int pos = { win_x + 1, win_y + 1 };

	return pos;
}

void window_set_mouse_cursor_position(uint32_t window, int32_t x, int32_t y) {
	/*if (!window_resources[window]->active) return;
	Window root, child;
	int32_t root_x, root_y;
	int32_t win_x, win_y;
	uint32_t mask;

	XQueryPointer(display, window_resources[window]->window, &root, &child, &root_x, &root_y, &win_x, &win_y, &mask);
	XWarpPointer(display, None, DefaultRootWindow(display), 0, 0, 0, 0, root_x - win_x + x + 2, root_y - win_y + window_resources[window]->window_height - y + 1);
	XFlush(display);*/
	return;
}

VkResult create_vulkan_surface(VkInstance instance, uint32_t window, VkSurfaceKHR* surface) {
	VkXlibSurfaceCreateInfoKHR create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	create_info.window = window_states[window].window;
	create_info.dpy = display;

	return vkCreateXlibSurfaceKHR(instance, &create_info, ((void*)0), surface);
}

VkResult destroy_vulkan_surface(VkInstance instance, VkSurfaceKHR surface) {
	vkDestroySurfaceKHR(instance, surface, 0);

	return VK_SUCCESS;
}

uint32_t window_process_next_event(struct window_event* event) {
	XEvent xevent;

	if (move_event) {
		move_event = 0;

		window_states[move_window].window_x_pos = move_x;
		window_states[move_window].window_y_pos = move_y;

		if (event) {
			event->type = WINDOW_EVENT_MOVE;
			event->window = move_window;
			event->info.window_event_move.x_position = move_x;
			event->info.window_event_move.y_position = move_y;
		}

		return 1;
	}

	int32_t is_trackable_window_event = 0;

	while (XPending(display)) {
		XNextEvent(display, &xevent);

		int32_t window_index = 0;
		for (; window_index < MAX_WINDOW_COUNT; window_index++) {
			if (window_states[window_index].window == xevent.xany.window && window_states[window_index].active) break;
		}
		if (window_index == MAX_WINDOW_COUNT) continue;

		switch (xevent.type) {

		case ConfigureNotify: {

			uint32_t new_width = (xevent.xconfigure.width > display_width ? display_width : xevent.xconfigure.width);
			uint32_t new_heigt = (xevent.xconfigure.height > display_height ? display_height : xevent.xconfigure.height);

			if (new_width != window_states[window_index].window_width || new_heigt != window_states[window_index].window_height) {
				is_trackable_window_event = 1;

				window_states[window_index].window_width = new_width;
				window_states[window_index].window_height = new_heigt;

				if (event) {
					event->type = WINDOW_EVENT_SIZE;
					event->info.window_event_size.width = new_width;
					event->info.window_event_size.height = new_heigt;
				}
			}

			uint32_t new_x_position = xevent.xconfigure.x;
			uint32_t new_y_position = xevent.xconfigure.y;

			if (new_x_position != window_states[window_index].window_x_pos || new_y_position != window_states[window_index].window_y_pos) {
				if (is_trackable_window_event = 1) {
					move_event = 1;
					move_window = window_index;
					move_x = new_x_position;
					move_y = new_y_position;
				}

				else {
					is_trackable_window_event = 1;

					if (event) {
						event->type = WINDOW_EVENT_MOVE;
						event->info.window_event_move.x_position = new_x_position;
						event->info.window_event_move.y_position = new_y_position;
					}
				}
			}
		
			
		} break;

		case ClientMessage: {
			if ((Atom)xevent.xclient.data.l[0] == wm_delete_window) {
				is_trackable_window_event = 1;

				window_states[window_index].active = 0;
				XDestroyWindow(display, window_states[window_index].window);

				if (event) event->type = WINDOW_EVENT_DESTROY;
			}
		} break;

		case ButtonPress: {

			if (xevent.xbutton.button == 4 || xevent.xbutton.button == 5) {
				is_trackable_window_event = 1;

				if (event) {
					event->type = WINDOW_EVENT_MOUSE_SCROLL;

					if (xevent.xbutton.button == 4) event->info.window_event_mouse_scroll.scroll_steps = 1;
					else event->info.window_event_mouse_scroll.scroll_steps = -1;
				}
			}	

		} break;

		case KeyPress: {

			KeySym keysym;
			uint32_t utf8_char = 0;
			uint32_t utf16_char = 0;
			XComposeStatus compose_status;

			int32_t len = Xutf8LookupString(window_states[window_index].ic, &xevent.xkey, utf8_char, sizeof(utf8_char), &keysym, &compose_status);

			if (len) {
				is_trackable_window_event = 1;
			}

		} break;

		}

		if (is_trackable_window_event) {
			if (event) event->window = window_index;
			return 1;
		}
	}

	return 0;
}


void platform_init() {
	XInitThreads();

	display = XOpenDisplay(NULL);
	if (display == NULL) {
		fprintf(stderr, "Unable to open X display\n");
		return;
	}

	screen = DefaultScreen(display);
	display_width = DisplayWidth(display, screen);
	display_height = DisplayHeight(display, screen);

	wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);

	xim = XOpenIM(display, NULL, NULL, NULL);

	return;
}

void platform_exit() {
	running = 0;
	XCloseIM(xim);
	XCloseDisplay(display);
}