#include "platform.h"

#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/Xlocale.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <malloc.h>

struct window_state 
{
	Window window;
	XIC ic;

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


void* dynamic_library_load(uint8_t* src) 
{

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

void (*dynamic_library_get_function(void* library_handle, uint8_t* function_name)) (void) 
{
	return dlsym(library_handle, function_name);
}

void dynamic_library_unload(void* library_handle) 
{
	dlclose(library_handle);
}

uint32_t directory_exists(uint8_t* path)
{

}

uint32_t create_directory(uint8_t* path)
{

}

uint32_t get_cwd(uint8_t* buffer, size_t buffer_size)
{

}

void show_console_window() { 
	return; 
}

void hide_console_window() { 
	return; 
}

void set_console_cursor_position(int32_t x, int32_t y) 
{
	printf("\033[%d;%dH", y, x);
}

void sleep_for_ms(uint32_t time_in_milliseconds) 
{
	usleep(time_in_milliseconds * 1000);
}

double get_time() 
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double)tv.tv_sec * 1000. + (double)tv.tv_usec / 1000.;
}

void* create_thread(void (address)(void*), void* args) 
{
	pthread_t* thread = malloc(sizeof(pthread_t));
	pthread_create(thread, NULL, (void* (*)(void*))address, args);
	return thread;
}

void join_thread(void* thread_handle) 
{
	pthread_join(*(pthread_t*)thread_handle, NULL);
	free(thread_handle);
}

uint32_t get_screen_width() 
{
	return display_width;
}

uint32_t get_screen_height() 
{
	return display_height;
}

uint32_t window_create(uint32_t posx, uint32_t posy, uint32_t width, uint32_t height, uint8_t* name, uint32_t visible) 
{

	uint32_t next_free_window_index = 0;
	for (; next_free_window_index < MAX_WINDOW_COUNT; next_free_window_index++) if (window_states[next_free_window_index].window == 0) break;
	if (next_free_window_index == MAX_WINDOW_COUNT) return WINDOW_CREATION_FAILED;

	Window window = XCreateSimpleWindow(display, RootWindow(display, screen), posx, posy, width, height, 1, BlackPixel(display, screen), WhitePixel(display, screen));

	XIC ic = XCreateIC(xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, window, NULL);

	XSelectInput(display, window, ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask | ButtonPressMask);
	XStoreName(display, window, name);
	XSetWMProtocols(display, window, &wm_delete_window, 1);
	if(visible) XMapWindow(display, window);

	window_states[next_free_window_index].window = window;
	window_states[next_free_window_index].ic = ic;
	window_states[next_free_window_index].window_width = width;
	window_states[next_free_window_index].window_height = height;
	window_states[next_free_window_index].window_x_pos = posx;
	window_states[next_free_window_index].window_y_pos = posy;

	return next_free_window_index;
}

uint32_t window_get_width(uint32_t window) 
{
	return window_states[window].window_width;
}

uint32_t window_get_height(uint32_t window) 
{
	return window_states[window].window_height;
}

uint32_t window_get_x_position(uint32_t window) 
{
	return window_states[window].window_x_pos;
}

uint32_t window_get_y_position(uint32_t window) 
{
	return window_states[window].window_y_pos;
}

int32_t window_is_selected(uint32_t window) 
{
	Window focused_window;
	int32_t revert_to;

	XGetInputFocus(display, &focused_window, &revert_to);

	return focused_window == window_states[window].window;
}

void window_destroy(uint32_t window) 
{

	XDestroyWindow(display, window_states[window].window);
	window_states[window].window = 0;
}

struct point2d_int window_get_mouse_cursor_position(uint32_t window) 
{
	Window root, child;
	int32_t root_x, root_y;
	int32_t win_x, win_y;
	uint32_t mask;

	XQueryPointer(display, window_states[window].window, &root, &child, &root_x, &root_y, &win_x, &win_y, &mask);

	struct point2d_int pos = { win_x + 1, win_y + 1 };
	if (pos.x > 0 && pos.x < window_states[window].window_width - 1 && pos.y > 0 && pos.y < window_states[window].window_height - 1) return pos;
	else return (struct point2d_int) { -1, -1 };
}

void window_set_mouse_cursor_position(uint32_t window, int32_t x, int32_t y) 
{
	return;
}

VkResult create_vulkan_surface(VkInstance instance, uint32_t window, VkSurfaceKHR* surface) 
{
	VkXlibSurfaceCreateInfoKHR create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	create_info.window = window_states[window].window;
	create_info.dpy = display;

	return vkCreateXlibSurfaceKHR(instance, &create_info, ((void*)0), surface);
}

VkResult destroy_vulkan_surface(VkInstance instance, VkSurfaceKHR surface) 
{
	vkDestroySurfaceKHR(instance, surface, 0);

	return VK_SUCCESS;
}

int match_window(Display* dpy, XEvent* event, XPointer arg) {
	Window target = *(Window*)arg;
	return (event->xany.window == target);
}

uint32_t window_process_next_event(uint32_t window, struct window_event* event)
{
	XEvent xevent;
	
	while (XCheckIfEvent(display, &xevent, match_window, (XPointer)&window_states[window].window))
	{
		switch (xevent.type) {

		case ConfigureNotify: {

			if (
				xevent.xconfigure.width  != window_states[window].window_width | 
				xevent.xconfigure.height != window_states[window].window_height |
				xevent.xconfigure.x      != window_states[window].window_x_pos |
				xevent.xconfigure.y      != window_states[window].window_y_pos
			)
			{
				window_states[window].window_width  = xevent.xconfigure.width;
				window_states[window].window_height = xevent.xconfigure.height;
				window_states[window].window_x_pos  = xevent.xconfigure.x;
				window_states[window].window_y_pos  = xevent.xconfigure.y;

				event->type = WINDOW_EVENT_MOVE_SIZE;
				event->info.event_move_size.width = xevent.xconfigure.width;
				event->info.event_move_size.height = xevent.xconfigure.height;
				event->info.event_move_size.position_x = xevent.xconfigure.x;
				event->info.event_move_size.position_y = xevent.xconfigure.y;

				return 1;
			}
		} break;

		case ClientMessage: {
			if ((Atom)xevent.xclient.data.l[0] == wm_delete_window)
			{
				event->type = WINDOW_EVENT_DESTROY;

				return 1;
			}
		} break;

		}
	}

	return 0;
}


void platform_init() 
{
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

void platform_exit() 
{
	XCloseIM(xim);
	XCloseDisplay(display);
}