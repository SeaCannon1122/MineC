#define VK_USE_PLATFORM_XLIB_KHR

#include <window/window.h>

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
#include <string.h>

#include <GL/glx.h>
#include <GL/glxext.h>


struct window_data_x11
{
	Window window;
	XIC ic;
	Colormap colormap;
	XVisualInfo* visual_info;
	Visual* visual;
	VisualID visualid;

	GLXContext glx_context;

	struct window_event dispatched_event;

	uint32_t width;
	uint32_t height;
	uint32_t position_x;
	uint32_t position_y;

	bool selected;

	void (*glXSwapIntervalEXT)(Display* dpy, GLXDrawable drawable, int interval);
};

struct window_context_x11
{
	Display* display;
	int screen;
	Atom wm_delete_window;
	XIM xim;

	struct
	{
		void* library;
		struct
		{
			PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
		} func;
	} vulkan;

	struct
	{
		void* library;
		struct
		{
			GLXFBConfig* (*glXGetFBConfigs)(Display* dpy, int screen, int* nelements);
			XVisualInfo* (*glXGetVisualFromFBConfig)(Display* dpy, GLXFBConfig config);

			PFNGLXGETPROCADDRESSPROC glXGetProcAddress;

			void (*glXDestroyContext)(Display* dpy, GLXContext ctx);
			Bool (*glXMakeCurrent)(Display* dpy, GLXDrawable drawable, GLXContext ctx);
			void (*glXSwapBuffers)(Display*, GLXDrawable);
		} func;

		struct window_data_x11* current_window;
	} opengl;
};

struct window_context_x11 context_memory;
struct window_context_x11* context = &context_memory;

bool window_init_context(void* transfered_context)
{
	if (transfered_context == NULL)
	{
		context->display = XOpenDisplay(NULL);
		context->screen = DefaultScreen(context->display);
		context->wm_delete_window = XInternAtom(context->display, "WM_DELETE_WINDOW", False);
		context->xim = XOpenIM(context->display, NULL, NULL, NULL);
	}
	else
		context = transfered_context;

	return 0;
}

void window_deinit_context()
{
	if (context == &context_memory)
	{
		XCloseIM(context->xim);
		XCloseDisplay(context->display);
	}
}

void* window_create(int32_t posx, int32_t posy, uint32_t width, uint32_t height, uint8_t* name, bool visible)
{
	struct window_data_x11* window_data;
	if ((window_data = malloc(sizeof(struct window_data_x11))) == NULL) return NULL;

	XVisualInfo vinfo_template;
	vinfo_template.screen = context->screen;
	vinfo_template.class = TrueColor;

	int vinfo_mask = VisualScreenMask | VisualClassMask;

	int nitems;
	window_data->visual_info = XGetVisualInfo(context->display, vinfo_mask, &vinfo_template, &nitems);
	if (!window_data->visual_info || nitems == 0) {
		free(window_data);
		return NULL;
	}

	window_data->visual = window_data->visual_info[0].visual;
	window_data->visualid = window_data->visual_info[0].visualid;
	int depth = window_data->visual_info[0].depth;

	window_data->colormap = XCreateColormap(context->display, RootWindow(context->display, context->screen), window_data->visual, AllocNone);

	XSetWindowAttributes swa;
	swa.colormap = window_data->colormap;
	swa.event_mask = ExposureMask | KeyPressMask;

	window_data->window = XCreateWindow(
		context->display, 
		RootWindow(context->display, context->screen),
		posx, posy, 
		width, height, 
		0, 
		depth, 
		InputOutput,
		window_data->visual,
		CWColormap | CWEventMask, 
		&swa
	);

	window_data->ic = XCreateIC(context->xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, window_data->window, NULL);

	XSelectInput(context->display, window_data->window, ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask | ButtonPressMask);
	XStoreName(context->display, window_data->window, name);
	XSetWMProtocols(context->display, window_data->window, &context->wm_delete_window, 1);
	if (visible) XMapWindow(context->display, window_data->window);

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
	
	XFree(window_data->visual_info);

	XDestroyIC(window_data->ic);
	XDestroyWindow(context->display, window_data->window);
	XFreeColormap(context->display, window_data->colormap);

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

	while (XCheckIfEvent(context->display, &xevent, match_window, (XPointer)&window_data->window))
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
			if ((Atom)xevent.xclient.data.l[0] == context->wm_delete_window)
			{
				window_data->dispatched_event.type = WINDOW_EVENT_DESTROY;

				return &window_data->dispatched_event;
			}
		} break;

		}
	}

	return NULL;
}

bool window_vulkan_load()
{
	if ((context->vulkan.library = dlopen("libvulkan.so.1", RTLD_NOW)) == NULL) return false;

	if ((context->vulkan.func.vkGetInstanceProcAddr = dlsym(context->vulkan.library, "vkGetInstanceProcAddr")) == NULL)
	{
		dlclose(context->vulkan.library);
		return false;
	}

	return true;
}

void window_vulkan_unload()
{
	dlclose(context->vulkan.library);
}

PFN_vkGetInstanceProcAddr window_get_vkGetInstanceProcAddr()
{
	return context->vulkan.func.vkGetInstanceProcAddr;
}

VkResult window_vkCreateSurfaceKHR(void* window, VkInstance instance, VkSurfaceKHR* surface)
{
	struct window_data_x11* window_data = window;

	PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHR_func = (PFN_vkCreateXlibSurfaceKHR)context->vulkan.func.vkGetInstanceProcAddr(instance, "vkCreateXlibSurfaceKHR");
	if (vkCreateXlibSurfaceKHR_func == NULL) return VK_ERROR_INITIALIZATION_FAILED;

	VkXlibSurfaceCreateInfoKHR create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	create_info.window = window_data->window;
	create_info.dpy = context->display;

	return vkCreateXlibSurfaceKHR_func(instance, &create_info, ((void*)0), surface);
}

uint8_t* window_get_VK_KHR_PLATFORM_SURFACE_EXTENSION_NAME()
{
	return VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
}


//opengl

bool window_opengl_load()
{
	if ((context->opengl.library = dlopen("libGL.so.1", RTLD_NOW | RTLD_NODELETE)) == NULL) return false;

	if ((context->opengl.func.glXGetProcAddress = dlsym(context->opengl.library, "glXGetProcAddress")) == NULL)
	{
		if ((context->opengl.func.glXGetProcAddress = dlsym(context->opengl.library, "glXGetProcAddressARB")) == NULL)
		{
			dlclose(context->opengl.library);
			return false;
		}
	}

	const char* (*glXQueryExtensionsString_func)(Display * dpy, int screen);
	const char* glx_extension_string;

	if ((glXQueryExtensionsString_func = dlsym(context->opengl.library, "glXQueryExtensionsString")) == NULL)
	{
		dlclose(context->opengl.library);
		return false;
	}
	else if ((glx_extension_string = glXQueryExtensionsString_func(context->display, context->screen)) == NULL)
	{
		dlclose(context->opengl.library);
		return false;
	}
	else if (strstr(glx_extension_string, "GLX_ARB_create_context") == NULL)
	{
		dlclose(context->opengl.library);
		return false;
	}

	context->opengl.func.glXGetFBConfigs = dlsym(context->opengl.library, "glXGetFBConfigs");
	context->opengl.func.glXGetVisualFromFBConfig = dlsym(context->opengl.library, "glXGetVisualFromFBConfig");
	context->opengl.func.glXGetProcAddress = dlsym(context->opengl.library, "glXGetProcAddress");
	context->opengl.func.glXDestroyContext = dlsym(context->opengl.library, "glXDestroyContext");
	context->opengl.func.glXMakeCurrent = dlsym(context->opengl.library, "glXMakeCurrent");
	context->opengl.func.glXSwapBuffers = dlsym(context->opengl.library, "glXSwapBuffers");

	if (
		context->opengl.func.glXGetFBConfigs == NULL ||
		context->opengl.func.glXGetVisualFromFBConfig == NULL ||
		context->opengl.func.glXGetProcAddress == NULL ||
		context->opengl.func.glXDestroyContext == NULL ||
		context->opengl.func.glXMakeCurrent == NULL ||
		context->opengl.func.glXSwapBuffers == NULL
	)
	{
		dlclose(context->opengl.library);
		return false;
	}

	return true;
}

void window_opengl_unload()
{
	dlclose(context->opengl.library);
}

bool window_glCreateContext(void* window, int32_t version_major, int32_t version_minor, void* share_window, bool* glSwapIntervalEXT_support)
{
	struct window_data_x11* window_data = window;
	struct window_data_x11* share_window_data = share_window;

	int fbcount;
	GLXFBConfig* fbc = context->opengl.func.glXGetFBConfigs(context->display, context->screen, &fbcount);
	if (!fbc || fbcount == 0) return false;

	GLXFBConfig bestFbc = NULL;
	for (int i = 0; i < fbcount; i++) {
		XVisualInfo* vi = context->opengl.func.glXGetVisualFromFBConfig(context->display, fbc[i]);
		if (vi) {
			if (vi->visualid == window_data->visualid) {
				bestFbc = fbc[i];
				XFree(vi);
				break;
			}
			XFree(vi);
		}
	}
	XFree(fbc);

	if (bestFbc == NULL) return false;


	PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB_func;
	if ((glXCreateContextAttribsARB_func = context->opengl.func.glXGetProcAddress("glXCreateContextAttribsARB")) == NULL) return false;

	int context_attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, version_major,
		GLX_CONTEXT_MINOR_VERSION_ARB, version_minor,
		GLX_CONTEXT_PROFILE_MASK_ARB,  GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_ROBUST_ACCESS_BIT_ARB,
		GLX_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB, GLX_LOSE_CONTEXT_ON_RESET_ARB,
		None
	};

	if ((window_data->glx_context = glXCreateContextAttribsARB_func(context->display, bestFbc, share_window ? share_window_data->glx_context : NULL, True, context_attribs)) == NULL) return false;
	if (context->opengl.func.glXMakeCurrent(context->display, window_data->window, window_data->glx_context) != True)
	{
		context->opengl.func.glXDestroyContext(context->display, window_data->glx_context);
		return false;
	}

	if ((window_data->glXSwapIntervalEXT = context->opengl.func.glXGetProcAddress("glXSwapIntervalEXT")) != NULL) *glSwapIntervalEXT_support = false;
	else *glSwapIntervalEXT_support = true;

	context->opengl.func.glXMakeCurrent(context->display, None, NULL);
	return true;
}

void window_glDestroyContext(void* window)
{
	struct window_data_x11* window_data = window;

	context->opengl.func.glXDestroyContext(context->display, window_data->glx_context);
}

bool window_glMakeCurrent(void* window)
{
	struct window_data_x11* window_data = window;

	bool result = true;

	if (window_data == NULL) result = (context->opengl.func.glXMakeCurrent(context->display, None, NULL) == True);
	else if ((result = (context->opengl.func.glXMakeCurrent(context->display, window_data->window, window_data->glx_context) == True)) == true) context->opengl.current_window = window;
	return result;
}

bool window_glSwapInterval(int interval)
{
	if (context->opengl.current_window->glXSwapIntervalEXT) return (context->opengl.current_window->glXSwapIntervalEXT(context->display, context->opengl.current_window->window, interval) == 0);
	else return false;
}

void (*window_glGetProcAddress(uint8_t* name)) (void)
{
	void (*function)(void);

	if ((function = context->opengl.func.glXGetProcAddress(name)) != NULL) return function;
	else return dlsym(context->opengl.library, name);
}

bool window_glSwapBuffers(void* window)
{
	context->opengl.func.glXSwapBuffers(context->display, ((struct window_data_x11*)(window))->window);
}