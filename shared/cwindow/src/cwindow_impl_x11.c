#define VK_USE_PLATFORM_XLIB_KHR

#include <window/window.h>

#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
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


struct cwindow_data_x11
{
	Window window;
	XIC ic;
	Colormap colormap;
	XVisualInfo* visual_info;
	Visual* visual;
	VisualID visualid;

	GLXContext glx_context;

	struct cwindow_event dispatched_event;

	uint32_t width;
	uint32_t height;
	uint32_t position_x;
	uint32_t position_y;

	bool selected;

	void (*glXSwapIntervalEXT)(Display* dpy, GLXDrawable drawable, int interval);
};

struct cwindow_context_x11
{
	Display* display;
	int screen;
	Atom wm_delete_window, _net_wm_icon;
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
			GLXFBConfig* (*glXChooseFBConfig)(Display* dpy, int screen, int* attrib_list, int* nelements);
			GLXFBConfig* (*glXGetFBConfigs)(Display* dpy, int screen, int* nelements);
			XVisualInfo* (*glXGetVisualFromFBConfig)(Display* dpy, GLXFBConfig config);

			PFNGLXGETPROCADDRESSPROC glXGetProcAddress;

			void (*glXDestroyContext)(Display* dpy, GLXContext ctx);
			Bool (*glXMakeCurrent)(Display* dpy, GLXDrawable drawable, GLXContext ctx);
			void (*glXSwapBuffers)(Display*, GLXDrawable);
		} func;

		struct cwindow_data_x11* current_window;
	} opengl;
};

struct cwindow_context_x11 context_memory;
struct cwindow_context_x11* context = &context_memory;

bool cwindow_init_context(void* transfered_context)
{
	if (transfered_context == NULL)
	{
		XInitThreads();

		context->display = XOpenDisplay(NULL);
		context->screen = DefaultScreen(context->display);
		context->wm_delete_window = XInternAtom(context->display, "WM_DELETE_WINDOW", False);
		context->_net_wm_icon = XInternAtom(context->display, "_NET_WM_ICON", False);
		context->xim = XOpenIM(context->display, NULL, NULL, NULL);
	}
	else
		context = transfered_context;

	return true;
}

void cwindow_deinit_context()
{
	if (context == &context_memory)
	{
		XCloseIM(context->xim);
		XCloseDisplay(context->display);
	}
}

void* cwindow_get_context()
{
	return context;
}

void* cwindow_create(int32_t posx, int32_t posy, uint32_t width, uint32_t height, uint8_t* name, bool visible)
{
	struct cwindow_data_x11* cwindow_data;
	if ((window_data = malloc(sizeof(struct cwindow_data_x11))) == NULL) return NULL;

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

	window_data->visual = cwindow_data->visual_info[0].visual;
	window_data->visualid = cwindow_data->visual_info[0].visualid;
	int depth = cwindow_data->visual_info[0].depth;

	window_data->colormap = XCreateColormap(context->display, RootWindow(context->display, context->screen), cwindow_data->visual, AllocNone);

	XSetWindowAttributes swa = { 0 };
	swa.colormap = cwindow_data->colormap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask | ButtonPressMask;

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

	window_data->ic = XCreateIC(context->xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, cwindow_data->window, NULL);

	XStoreName(context->display, cwindow_data->window, name);
	XSetWMProtocols(context->display, cwindow_data->window, &context->wm_delete_window, 1);
	if (visible) XMapWindow(context->display, cwindow_data->window);

	window_data->width = width;
	window_data->height = height;
	window_data->position_x = posx;
	window_data->position_y = posy;
	window_data->selected = false;

	return cwindow_data;
}

void cwindow_destroy(void* window)
{
	struct cwindow_data_x11* cwindow_data = window;
	
	XFree(window_data->visual_info);

	XDestroyIC(window_data->ic);
	XDestroyWindow(context->display, cwindow_data->window);
	XFreeColormap(context->display, cwindow_data->colormap);

	free(window_data);
}

bool cwindow_set_icon(void* window, uint32_t* icon_rgba_pixel_data, uint32_t icon_width, uint32_t icon_height)
{
	struct cwindow_data_x11* cwindow_data = window;

	size_t data_len = 2 + (icon_width * icon_height);
	uint32_t* data = malloc(sizeof(uint32_t) * data_len);
	if (data == NULL) return false;

	data[0] = icon_width;
	data[1] = icon_height;

	for (uint32_t i = 0; i < icon_width * icon_height; i++)
	{
		data[2 + i] = (icon_rgba_pixel_data[i] & 0xff00ff00) | ((icon_rgba_pixel_data[i] & 0xff) << 16) | ((icon_rgba_pixel_data[i] & 0xff0000) >> 16);
	}

	XChangeProperty(context->display, cwindow_data->window, context->_net_wm_icon, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)data, data_len);
	XFlush(context->display);
	free(data);

	return true;
}

void cwindow_get_dimensions(void* window, uint32_t* width, uint32_t* height, int32_t* screen_position_x, int32_t* screen_position_y)
{
	struct cwindow_data_x11* cwindow_data = window;

	*width = cwindow_data->width;
	*height = cwindow_data->height;
	*screen_position_x = cwindow_data->position_x;
	*screen_position_y = cwindow_data->position_y;
}

bool cwindow_is_selected(void* window)
{
	struct cwindow_data_x11* cwindow_data = window;
	return cwindow_data->selected;
}

void cwindow_get_mouse_cursor_position(void* window, int32_t* position_x, int32_t* position_y)
{

}

void cwindow_set_mouse_cursor_position(void* window, int32_t x, int32_t y)
{

}

int match_window(Display* dpy, XEvent* event, XPointer arg) {
	Window target = *(Window*)arg;
	return (event->xany.window == target);
}

struct cwindow_event* cwindow_next_event(void* window)
{
	struct cwindow_data_x11* cwindow_data = window;

	XEvent xevent;

	while (XCheckIfEvent(context->display, &xevent, match_window, (XPointer)&window_data->window))
	{
		switch (xevent.type) {

		case ConfigureNotify: {

			if (
				xevent.xconfigure.width != cwindow_data->width |
				xevent.xconfigure.height != cwindow_data->height |
				xevent.xconfigure.x != cwindow_data->position_x |
				xevent.xconfigure.y != cwindow_data->position_y
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

bool cwindow_vulkan_load()
{
	if ((context->vulkan.library = dlopen("libvulkan.so.1", RTLD_NOW)) == NULL) return false;

	if ((context->vulkan.func.vkGetInstanceProcAddr = dlsym(context->vulkan.library, "vkGetInstanceProcAddr")) == NULL)
	{
		dlclose(context->vulkan.library);
		return false;
	}

	return true;
}

void cwindow_vulkan_unload()
{
	dlclose(context->vulkan.library);
}

PFN_vkGetInstanceProcAddr cwindow_get_vkGetInstanceProcAddr()
{
	return context->vulkan.func.vkGetInstanceProcAddr;
}

VkResult cwindow_vkCreateSurfaceKHR(void* window, VkInstance instance, VkSurfaceKHR* surface)
{
	struct cwindow_data_x11* cwindow_data = window;

	PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHR_func = (PFN_vkCreateXlibSurfaceKHR)context->vulkan.func.vkGetInstanceProcAddr(instance, "vkCreateXlibSurfaceKHR");
	if (vkCreateXlibSurfaceKHR_func == NULL) return VK_ERROR_INITIALIZATION_FAILED;

	VkXlibSurfaceCreateInfoKHR create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	create_info.window = cwindow_data->window;
	create_info.dpy = context->display;

	return vkCreateXlibSurfaceKHR_func(instance, &create_info, ((void*)0), surface);
}

uint8_t* cwindow_get_VK_KHR_PLATFORM_SURFACE_EXTENSION_NAME()
{
	return VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
}


//opengl

bool cwindow_opengl_load()
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

	context->opengl.func.glXChooseFBConfig = dlsym(context->opengl.library, "glXChooseFBConfig");
	context->opengl.func.glXGetFBConfigs = dlsym(context->opengl.library, "glXGetFBConfigs");
	context->opengl.func.glXGetVisualFromFBConfig = dlsym(context->opengl.library, "glXGetVisualFromFBConfig");
	context->opengl.func.glXGetProcAddress = dlsym(context->opengl.library, "glXGetProcAddress");
	context->opengl.func.glXDestroyContext = dlsym(context->opengl.library, "glXDestroyContext");
	context->opengl.func.glXMakeCurrent = dlsym(context->opengl.library, "glXMakeCurrent");
	context->opengl.func.glXSwapBuffers = dlsym(context->opengl.library, "glXSwapBuffers");

	if (
		context->opengl.func.glXChooseFBConfig == NULL ||
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

void cwindow_opengl_unload()
{
	dlclose(context->opengl.library);
}

bool cwindow_glCreateContext(void* window, int32_t version_major, int32_t version_minor, void* share_window, bool* glSwapIntervalEXT_support)
{
	struct cwindow_data_x11* cwindow_data = window;
	struct cwindow_data_x11* share_window_data = share_window;

	int fbcount = 0;
	int fb_attribs[] = {
		GLX_X_RENDERABLE, True,
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
		GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8, GLX_ALPHA_SIZE, 8,
		GLX_DEPTH_SIZE, 24, GLX_STENCIL_SIZE, 8,
		GLX_DOUBLEBUFFER, True,
		None
	};

	GLXFBConfig* fbc = context->opengl.func.glXChooseFBConfig(context->display, context->screen, fb_attribs, &fbcount);
	if (!fbc || fbcount == 0) return false;

	GLXFBConfig bestFbc = NULL;
	for (int i = 0; i < fbcount; i++) {
		XVisualInfo* vi = context->opengl.func.glXGetVisualFromFBConfig(context->display, fbc[i]);
		if (vi) {
			if (vi->visualid == cwindow_data->visualid) {
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
	if ((glXCreateContextAttribsARB_func = (PFNGLXCREATECONTEXTATTRIBSARBPROC)context->opengl.func.glXGetProcAddress("glXCreateContextAttribsARB")) == NULL) return false;

	int context_attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, version_major,
		GLX_CONTEXT_MINOR_VERSION_ARB, version_minor,
		GLX_CONTEXT_PROFILE_MASK_ARB,  GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_ROBUST_ACCESS_BIT_ARB,
		GLX_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB, GLX_LOSE_CONTEXT_ON_RESET_ARB,
		None
	};

	if ((window_data->glx_context = glXCreateContextAttribsARB_func(context->display, bestFbc, share_window ? share_window_data->glx_context : NULL, True, context_attribs)) == NULL) return false;
	if (context->opengl.func.glXMakeCurrent(context->display, cwindow_data->window, cwindow_data->glx_context) != True)
	{
		context->opengl.func.glXDestroyContext(context->display, cwindow_data->glx_context);
		return false;
	}

	if ((window_data->glXSwapIntervalEXT = (void (*)(Display * dpy, GLXDrawable drawable, int interval))context->opengl.func.glXGetProcAddress("glXSwapIntervalEXT")) == NULL) *glSwapIntervalEXT_support = false;
	else *glSwapIntervalEXT_support = true;

	context->opengl.func.glXMakeCurrent(context->display, None, NULL);
	return true;
}

bool cwindow_glDestroyContext(void* window)
{
	struct cwindow_data_x11* cwindow_data = window;

	context->opengl.func.glXDestroyContext(context->display, cwindow_data->glx_context);

	return true;
}

bool cwindow_glMakeCurrent(void* window)
{
	struct cwindow_data_x11* cwindow_data = window;

	bool result = true;

	if (window_data == NULL) result = (context->opengl.func.glXMakeCurrent(context->display, None, NULL) == True);
	else if ((result = (context->opengl.func.glXMakeCurrent(context->display, cwindow_data->window, cwindow_data->glx_context) == True)) == true) context->opengl.current_window = window;
	return result;
}

bool cwindow_glSwapIntervalEXT(int interval)
{
	if (context->opengl.current_window->glXSwapIntervalEXT == NULL) return false;

	context->opengl.current_window->glXSwapIntervalEXT(context->display, context->opengl.current_window->window, interval);
	return true;
}

void (*window_glGetProcAddress(uint8_t* name)) (void)
{
	void (*function)(void);

	if ((function = context->opengl.func.glXGetProcAddress(name)) != NULL) return function;
	else return dlsym(context->opengl.library, name);
}

bool cwindow_glSwapBuffers(void* window)
{
	context->opengl.func.glXSwapBuffers(context->display, ((struct cwindow_data_x11*)(window))->window);
}