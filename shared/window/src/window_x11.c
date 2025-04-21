#define VK_USE_PLATFORM_XLIB_KHR

#include "window.h"

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

#ifdef _WINDOW_SUPPORT_OPENGL

#include <GL/gl.h>
#include <GL/glx.h>

#endif

Display* display;
Atom wm_delete_window;
XIM xim;

struct window_data_x11
{
	Window window;
	XIC ic;

#ifdef _WINDOW_SUPPORT_OPENGL
	GLXContext glx_context;
	GLXFBConfig* fbc;
#endif

	struct window_event dispatched_event;

	uint32_t width;
	uint32_t height;
	uint32_t position_x;
	uint32_t position_y;

	bool selected;
};

void window_init_system()
{
	display = XOpenDisplay(NULL);
	wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
	xim = XOpenIM(display, NULL, NULL, NULL);
}

void window_deinit_system()
{
	XCloseIM(xim);
	XCloseDisplay(display);
}

void* window_create(int32_t posx, int32_t posy, uint32_t width, uint32_t height, uint8_t* name, bool visible, struct window_opengL_format_description* opengl_format_description)
{
	struct window_data_x11* window_data = malloc(sizeof(struct window_data_x11));

#ifdef _WINDOW_SUPPORT_OPENGL
	static int visual_attribs[] = {
		GLX_X_RENDERABLE, True,
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_RENDER_TYPE,   GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
		GLX_RED_SIZE,      8,
		GLX_GREEN_SIZE,    8,
		GLX_BLUE_SIZE,     8,
		GLX_ALPHA_SIZE,    8,
		GLX_DEPTH_SIZE,    24,
		GLX_STENCIL_SIZE,  8,
		GLX_DOUBLEBUFFER,  True,
		None
	};

	int fbcount;
	window_data->fbc = glXChooseFBConfig(display, DefaultScreen(display), visual_attribs, &fbcount);

	XVisualInfo* vi = glXGetVisualFromFBConfig(display, window_data->fbc[0]);

	XSetWindowAttributes swa;
	swa.colormap = XCreateColormap(display, RootWindow(display, vi->screen), vi->visual, AllocNone);
	swa.event_mask = ExposureMask | KeyPressMask;
	Window window = XCreateWindow(display, RootWindow(display, vi->screen),
		posx, posy, width, height, 0, vi->depth, InputOutput,
		vi->visual, CWColormap | CWEventMask, &swa);
#else

	Window window = XCreateSimpleWindow(display, DefaultRootWindow(display), posx, posy, width, height, 0, BlackPixel(display, 0), WhitePixel(display, 0));

#endif

	XIC ic = XCreateIC(xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, window, NULL);

	XSelectInput(display, window, ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask | ButtonPressMask);
	XStoreName(display, window, name);
	XSetWMProtocols(display, window, &wm_delete_window, 1);
	if (visible) XMapWindow(display, window);

	window_data->window = window;
	window_data->ic = ic;
	
	window_data->width = width;
	window_data->height = height;
	window_data->position_x = posx;
	window_data->position_y = posy;
	window_data->selected = false;

	return window_data;
}

void window_destroy(void* window)
{
	struct window_data_x11* window_data = window;

#ifdef _WINDOW_SUPPORT_OPENGL
	XFree(window_data->fbc);
#endif


	XDestroyIC(window_data->ic);
	XDestroyWindow(display, window_data->window);

	free(window_data);
}

void window_get_dimensions(void* window, uint32_t* width, uint32_t* height, int32_t* screen_position_x, int32_t* screen_position_y)
{
	struct window_data_x11* window_data = window;

	*width = window_data->width;
	*height = window_data->height;
	*screen_position_x = window_data->position_x;
	*screen_position_y = window_data->position_y;
}

bool window_is_selected(void* window)
{
	struct window_data_x11* window_data = window;
	return window_data->selected;
}

void window_get_mouse_cursor_position(void* window, int32_t* position_x, int32_t* position_y)
{

}

void window_set_mouse_cursor_position(void* window, int32_t x, int32_t y)
{

}

int match_window(Display* dpy, XEvent* event, XPointer arg) {
	Window target = *(Window*)arg;
	return (event->xany.window == target);
}

struct window_event* window_next_event(void* window)
{
	struct window_data_x11* window_data = window;

	XEvent xevent;

	while (XCheckIfEvent(display, &xevent, match_window, (XPointer)&window_data->window))
	{
		switch (xevent.type) {

		case ConfigureNotify: {

			if (
				xevent.xconfigure.width != window_data->width |
				xevent.xconfigure.height != window_data->height |
				xevent.xconfigure.x != window_data->position_x |
				xevent.xconfigure.y != window_data->position_y
				)
			{
				window_data->width = xevent.xconfigure.width;
				window_data->height = xevent.xconfigure.height;
				window_data->position_x = xevent.xconfigure.x;
				window_data->position_y = xevent.xconfigure.y;

				window_data->dispatched_event.type = WINDOW_EVENT_MOVE_SIZE;
				window_data->dispatched_event.info.move_size.width = xevent.xconfigure.width;
				window_data->dispatched_event.info.move_size.height = xevent.xconfigure.height;
				window_data->dispatched_event.info.move_size.position_x = xevent.xconfigure.x;
				window_data->dispatched_event.info.move_size.position_y = xevent.xconfigure.y;

				return &window_data->dispatched_event;
			}
		} break;

		case ClientMessage: {
			if ((Atom)xevent.xclient.data.l[0] == wm_delete_window)
			{
				window_data->dispatched_event.type = WINDOW_EVENT_DESTROY;

				return &window_data->dispatched_event;
			}
		} break;

		}
	}

	return NULL;
}

#ifdef _WINDOW_SUPPORT_VULKAN

VkResult window_vkCreateSurfaceKHR(void* window, VkInstance instance, VkSurfaceKHR* surface)
{
	struct window_data_x11* window_data = window;

	VkXlibSurfaceCreateInfoKHR create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	create_info.window = window_data->window;
	create_info.dpy = display;

	return vkCreateXlibSurfaceKHR(instance, &create_info, ((void*)0), surface);
}

uint8_t* window_get_vk_khr_surface_extension_name()
{
	return VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
}

#endif 

#ifdef _WINDOW_SUPPORT_OPENGL

typedef GLXContext(*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

bool window_opengl_context_create(void* window, int32_t version_major, int32_t version_minor, void* share_window)
{
	struct window_data_x11* window_data = window;
	struct window_data_x11* share_window_data = share_window;

	glXCreateContextAttribsARBProc glXCreateContextAttribsARB = glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");

	int context_attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, version_major,
		GLX_CONTEXT_MINOR_VERSION_ARB, version_minor,
		GLX_CONTEXT_PROFILE_MASK_ARB,  GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		None
	};

	window_data->glx_context = glXCreateContextAttribsARB(display, window_data->fbc[0], share_window ? share_window_data->glx_context : NULL, True, context_attribs);
}

void window_opengl_context_destroy(void* window)
{
	struct window_data_x11* window_data = window;

	glXDestroyContext(display, window_data->glx_context);
}

void window_opengl_context_make_current(void* window)
{
	struct window_data_x11* window_data = window;

	glXMakeCurrent(display, window_data->window, window_data->glx_context);
}

typedef int (*glXSwapIntervalSGIProc)(int);

void window_opengl_set_vsync(bool vsync)
{
	glXSwapIntervalSGIProc glXSwapIntervalSGI = glXGetProcAddress((const GLubyte*)"glXSwapIntervalSGI");

	glXSwapIntervalSGI(vsync ? 1 : 0);
}

void window_opengl_swap_buffers(void* window)
{
	struct window_data_x11* window_data = window;

	glXSwapBuffers(display, window_data->window);
}

#endif