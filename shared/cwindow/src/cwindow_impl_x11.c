#include <cwindow/cwindow.h>

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

struct cwindow_context
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
	} opengl;
};

struct cwindow
{
	cwindow_context* context;

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
	uint32_t posx;
	uint32_t posy;

	bool selected;

	void (*glXSwapIntervalEXT)(Display* dpy, GLXDrawable drawable, int interval);
};

cwindow_context* cwindow_context_create(const uint8_t* name)
{
	bool
		success = true,
		context_memory_allocated = false,
		display_opened = false,
		im_opened = false
	;

	cwindow_context* context = malloc(sizeof(cwindow_context));
	if (context == NULL) success = false;
	else context_memory_allocated = true;

	if (XInitThreads() == 0) success = false;

	if ((context->display = XOpenDisplay(NULL)) == NULL) success = false;
	else display_opened = true;

	context->screen = DefaultScreen(context->display);
	context->wm_delete_window = XInternAtom(context->display, "WM_DELETE_WINDOW", False);
	context->_net_wm_icon = XInternAtom(context->display, "_NET_WM_ICON", False);

	if ((context->xim = XOpenIM(context->display, NULL, NULL, NULL)) == NULL) success = false;
	else im_opened = true;

	if (success) return context;
	else
	{
		if (im_opened) XCloseIM(context->xim);
		if (display_opened) XCloseDisplay(context->display);
		if (context_memory_allocated) free(context);

		return NULL;
	}
}

void cwindow_context_destroy(cwindow_context* context)
{
	XCloseIM(context->xim);
	XCloseDisplay(context->display);
	free(context);
}

void cwindow_context_get_display_dimensions(cwindow_context* context, uint32_t* width, uint32_t* height)
{

}

cwindow* cwindow_create(cwindow_context* context, int32_t posx, int32_t posy, uint32_t width, uint32_t height, const uint8_t* name, bool visible)
{
	bool
		success = true,
		window_memory_allocated = false,
		got_visual_info = false,
		created_colormap = false,
		created_window = false,
		created_ic = false
	;

	cwindow* window;
	if ((window = malloc(sizeof(struct cwindow))) == NULL) success = false;
	else window_memory_allocated = true;

	XVisualInfo vinfo_template;
	vinfo_template.screen = context->screen;
	vinfo_template.class = TrueColor;

	int vinfo_mask = VisualScreenMask | VisualClassMask;

	int nitems;
	window->visual_info = XGetVisualInfo(context->display, vinfo_mask, &vinfo_template, &nitems);
	if (!window->visual_info || nitems == 0) {
		free(window);
		return NULL;
	}

	window->visual = window->visual_info[0].visual;
	window->visualid = window->visual_info[0].visualid;
	int depth = window->visual_info[0].depth;

	window->colormap = XCreateColormap(context->display, RootWindow(context->display, context->screen), window->visual, AllocNone);

	XSetWindowAttributes swa = { 0 };
	swa.colormap = window->colormap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask | ButtonPressMask;

	window->window = XCreateWindow(
		context->display, 
		RootWindow(context->display, context->screen),
		posx, posy, 
		width, height, 
		0, 
		depth, 
		InputOutput,
		window->visual,
		CWColormap | CWEventMask, 
		&swa
	);

	window->ic = XCreateIC(context->xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, window->window, NULL);

	XStoreName(context->display, window->window, name);
	XSetWMProtocols(context->display, window->window, &context->wm_delete_window, 1);
	if (visible) XMapWindow(context->display, window->window);

	if (success)
	{
		window->context = context;
		window->width = width;
		window->height = height;
		window->posx = posx;
		window->posy = posy;
		window->selected = false;

		return window;
	}
	else
	{
		if (window_memory_allocated) free(window);

		return NULL;
	}
}

void cwindow_destroy(cwindow* window)
{	
	XFree(window->visual_info);

	XDestroyIC(window->ic);
	XDestroyWindow(window->context->display, window->window);
	XFreeColormap(window->context->display, window->colormap);

	free(window);
}

bool cwindow_set_icon(cwindow* window, const uint32_t* icon_rgba_pixel_data, uint32_t icon_width, uint32_t icon_height)
{
	size_t data_len = 2 + (icon_width * icon_height);
	uint32_t* data = malloc(sizeof(uint32_t) * data_len);
	if (data == NULL) return false;

	data[0] = icon_width;
	data[1] = icon_height;

	for (uint32_t i = 0; i < icon_width * icon_height; i++)
	{
		data[2 + i] = (icon_rgba_pixel_data[i] & 0xff00ff00) | ((icon_rgba_pixel_data[i] & 0xff) << 16) | ((icon_rgba_pixel_data[i] & 0xff0000) >> 16);
	}

	XChangeProperty(window->context->display, window->window, window->context->_net_wm_icon, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)data, data_len);
	XFlush(window->context->display);
	free(data);

	return true;
}

void cwindow_get_dimensions(cwindow* window, uint32_t* width, uint32_t* height, int32_t* posx, int32_t* posy)
{
	*width = window->width;
	*height = window->height;
	*posx = window->posx;
	*posy = window->posy;
}

bool cwindow_is_selected(cwindow* window)
{
	return window->selected;
}

void cwindow_get_mouse_cursor_position(cwindow* window, int32_t* posx, int32_t* posy)
{

}

void cwindow_set_mouse_cursor_position(cwindow* window, int32_t posx, int32_t posy)
{

}

int match_window(Display* dpy, XEvent* event, XPointer arg) {
	Window target = *(Window*)arg;
	return (event->xany.window == target);
}

const cwindow_event* cwindow_next_event(cwindow* window)
{
	XEvent xevent;

	while (XCheckIfEvent(window->context->display, &xevent, match_window, (XPointer)&window->window))
	{
		switch (xevent.type) {

		case ConfigureNotify: {

			if (
				xevent.xconfigure.width != window->width |
				xevent.xconfigure.height != window->height |
				xevent.xconfigure.x != window->posx |
				xevent.xconfigure.y != window->posy
				)
			{
				window->width = xevent.xconfigure.width;
				window->height = xevent.xconfigure.height;
				window->posx = xevent.xconfigure.x;
				window->posy = xevent.xconfigure.y;

				window->dispatched_event.type = CWINDOW_EVENT_MOVE_SIZE;
				window->dispatched_event.info.move_size.width = xevent.xconfigure.width;
				window->dispatched_event.info.move_size.height = xevent.xconfigure.height;
				window->dispatched_event.info.move_size.position_x = xevent.xconfigure.x;
				window->dispatched_event.info.move_size.position_y = xevent.xconfigure.y;

				return &window->dispatched_event;
			}
		} break;

		case ClientMessage: {
			if ((Atom)xevent.xclient.data.l[0] == window->context->wm_delete_window)
			{
				window->dispatched_event.type = CWINDOW_EVENT_DESTROY;

				return &window->dispatched_event;
			}
		} break;

		}
	}

	return NULL;
}

void cwindow_freeze_event_queue(cwindow* window)
{

}

void cwindow_unfreeze_event_queue(cwindow* window)
{

}

Display* window_impl_x11_context_get_display(cwindow_context* context)
{

}

Window window_impl_x11_get_window(cwindow* window)
{

}

bool cwindow_context_graphics_vulkan_load(cwindow_context* context, PFN_vkGetInstanceProcAddr* gpa)
{
	if ((context->vulkan.library = dlopen("libvulkan.so.1", RTLD_NOW)) == NULL) return false;

	if ((context->vulkan.func.vkGetInstanceProcAddr = dlsym(context->vulkan.library, "vkGetInstanceProcAddr")) == NULL)
	{
		dlclose(context->vulkan.library);
		return false;
	}

	*gpa = context->vulkan.func.vkGetInstanceProcAddr;
	return true;
}

void cwindow_context_graphics_vulkan_unload(cwindow_context* context)
{
	dlclose(context->vulkan.library);
}

VkResult cwindow_vkCreateSurfaceKHR(cwindow* window, VkInstance instance, VkSurfaceKHR* surface)
{
	PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHR_func = (PFN_vkCreateXlibSurfaceKHR)window->context->vulkan.func.vkGetInstanceProcAddr(instance, "vkCreateXlibSurfaceKHR");
	if (vkCreateXlibSurfaceKHR_func == NULL) return VK_ERROR_INITIALIZATION_FAILED;

	VkXlibSurfaceCreateInfoKHR create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	create_info.window = window->window;
	create_info.dpy = window->context->display;

	return vkCreateXlibSurfaceKHR_func(instance, &create_info, ((void*)0), surface);
}

//opengl

bool cwindow_context_graphics_opengl_load(cwindow_context* context)
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

void cwindow_context_graphics_opengl_unload(cwindow_context* context)
{
	dlclose(context->opengl.library);
}

bool cwindow_glCreateContext(cwindow* window, int32_t version_major, int32_t version_minor, cwindow* share_window, bool* glSwapIntervalEXT_support)
{
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

	GLXFBConfig* fbc = window->context->opengl.func.glXChooseFBConfig(window->context->display, window->context->screen, fb_attribs, &fbcount);
	if (!fbc || fbcount == 0) return false;

	GLXFBConfig bestFbc = NULL;
	for (int i = 0; i < fbcount; i++) {
		XVisualInfo* vi = window->context->opengl.func.glXGetVisualFromFBConfig(window->context->display, fbc[i]);
		if (vi) {
			if (vi->visualid == window->visualid) {
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
	if ((glXCreateContextAttribsARB_func = (PFNGLXCREATECONTEXTATTRIBSARBPROC)window->context->opengl.func.glXGetProcAddress("glXCreateContextAttribsARB")) == NULL) return false;

	int context_attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, version_major,
		GLX_CONTEXT_MINOR_VERSION_ARB, version_minor,
		GLX_CONTEXT_PROFILE_MASK_ARB,  GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_ROBUST_ACCESS_BIT_ARB,
		GLX_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB, GLX_LOSE_CONTEXT_ON_RESET_ARB,
		None
	};

	if ((window->glx_context = glXCreateContextAttribsARB_func(window->context->display, bestFbc, share_window ? share_window->glx_context : NULL, True, context_attribs)) == NULL) return false;
	if (window->context->opengl.func.glXMakeCurrent(window->context->display, window->window, window->glx_context) != True)
	{
		window->context->opengl.func.glXDestroyContext(window->context->display, window->glx_context);
		return false;
	}

	if ((window->glXSwapIntervalEXT = (void (*)(Display * dpy, GLXDrawable drawable, int interval))window->context->opengl.func.glXGetProcAddress("glXSwapIntervalEXT")) == NULL) *glSwapIntervalEXT_support = false;
	else *glSwapIntervalEXT_support = true;

	window->context->opengl.func.glXMakeCurrent(window->context->display, None, NULL);
	return true;
}

void cwindow_glDestroyContext(cwindow* window)
{
	window->context->opengl.func.glXDestroyContext(window->context->display, window->glx_context);

	return true;
}

bool cwindow_glMakeCurrent(cwindow* window, bool current)
{
	return (window->context->opengl.func.glXMakeCurrent(window->context->display, current ? window->window : None, current ? window->glx_context : NULL) == True);
}

bool cwindow_glSwapIntervalEXT(cwindow* window, int interval)
{
	if (window->glXSwapIntervalEXT == NULL) return false;

	window->glXSwapIntervalEXT(window->context->display, window->window, interval);
	return true;
}

void (*cwindow_glGetProcAddress(cwindow* window, const uint8_t* name)) (void)
{
	void (*function)(void);

	if ((function = window->context->opengl.func.glXGetProcAddress(name)) != NULL) return function;
	else return dlsym(window->context->opengl.library, name);
}

bool cwindow_glSwapBuffers(cwindow* window)
{
	window->context->opengl.func.glXSwapBuffers(window->context->display, window->window);
}
