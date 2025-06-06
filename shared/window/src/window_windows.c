#define VK_USE_PLATFORM_WIN32_KHR

#include <window/window.h>
#include <GL/wgl.h>

#include <stdio.h>
#include <stdlib.h>

#define _WINDOW_QUEUE_EXTENSION_EVENTS_COUNT 256

struct window_data_windows
{
	HWND hwnd;

	HDC hdc;
	HGLRC hglrc;
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

	struct window_event dispatched_event;
	bool move_size;

	uint32_t width;
	uint32_t height;
	uint32_t position_x;
	uint32_t position_y;

	bool selected;

	struct window_event* event_queue;
	uint32_t event_queue_length;
	uint32_t free_event_queue_index;
	uint32_t last_event_queue_index;
};

struct window_context_windows
{
	struct
	{
		HMODULE library;
		struct
		{
			PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
		} func;
	} vulkan;

	struct
	{
		HMODULE library;
		struct
		{
			PFNWGLCREATECONTEXTPROC wglCreateContext;
			PFNWGLDELETECONTEXTPROC wglDeleteContext;
			PFNWGLMAKECURRENTPROC wglMakeCurrent;
			PFNWGLGETPROCADDRESSPROC wglGetProcAddress;
		} func;

		struct window_data_windows* current_window;
	} opengl;
};

struct window_context_windows context_memory;
struct window_context_windows* context = &context_memory;

void _window_event_queue_add(struct window_data_windows* window_data, struct window_event* event)
{
	if ((window_data->free_event_queue_index + 1) % window_data->event_queue_length == window_data->last_event_queue_index)
	{
		void* new_event_queue = realloc(window_data->event_queue, (window_data->event_queue_length + _WINDOW_QUEUE_EXTENSION_EVENTS_COUNT) * sizeof(struct window_event));
		if (new_event_queue == NULL)
		{
			window_data->event_queue[window_data->last_event_queue_index].type = WINDOW_EVENT_DESTROY;
			return;
		}

		window_data->event_queue = new_event_queue;

		if (window_data->free_event_queue_index != window_data->event_queue_length - 1)
		{
			memmove(
				&window_data->event_queue[window_data->last_event_queue_index + _WINDOW_QUEUE_EXTENSION_EVENTS_COUNT],
				&window_data->event_queue[window_data->last_event_queue_index],
				(window_data->event_queue_length - window_data->last_event_queue_index) * sizeof(struct window_event)
			);
			window_data->last_event_queue_index += _WINDOW_QUEUE_EXTENSION_EVENTS_COUNT;
		}

		window_data->event_queue_length += _WINDOW_QUEUE_EXTENSION_EVENTS_COUNT;
	}

	window_data->event_queue[window_data->free_event_queue_index] = *event;
	window_data->free_event_queue_index = (window_data->free_event_queue_index + 1) % window_data->event_queue_length;
}

int32_t _map_key(int32_t key) 
{

	switch (key) {

	case '1': return WINDOW_KEY_1;
	case '2': return WINDOW_KEY_2;
	case '3': return WINDOW_KEY_3;
	case '4': return WINDOW_KEY_4;
	case '5': return WINDOW_KEY_5;
	case '6': return WINDOW_KEY_6;
	case '7': return WINDOW_KEY_7;
	case '8': return WINDOW_KEY_8;
	case '9': return WINDOW_KEY_9;
	case '0': return WINDOW_KEY_0;

	case 'A': return WINDOW_KEY_A;
	case 'B': return WINDOW_KEY_B;
	case 'C': return WINDOW_KEY_C;
	case 'D': return WINDOW_KEY_D;
	case 'E': return WINDOW_KEY_E;
	case 'F': return WINDOW_KEY_F;
	case 'G': return WINDOW_KEY_G;
	case 'H': return WINDOW_KEY_H;
	case 'I': return WINDOW_KEY_I;
	case 'J': return WINDOW_KEY_J;
	case 'K': return WINDOW_KEY_K;
	case 'L': return WINDOW_KEY_L;
	case 'M': return WINDOW_KEY_M;
	case 'N': return WINDOW_KEY_N;
	case 'O': return WINDOW_KEY_O;
	case 'P': return WINDOW_KEY_P;
	case 'Q': return WINDOW_KEY_Q;
	case 'R': return WINDOW_KEY_R;
	case 'S': return WINDOW_KEY_S;
	case 'T': return WINDOW_KEY_T;
	case 'U': return WINDOW_KEY_U;
	case 'V': return WINDOW_KEY_V;
	case 'W': return WINDOW_KEY_W;
	case 'X': return WINDOW_KEY_X;
	case 'Y': return WINDOW_KEY_Y;
	case 'Z': return WINDOW_KEY_Z;

	case VK_ADD: return WINDOW_KEY_PLUS;
	case VK_SUBTRACT: return WINDOW_KEY_MINUS;

	case VK_F1: return WINDOW_KEY_F1;
	case VK_F2: return WINDOW_KEY_F2;
	case VK_F3: return WINDOW_KEY_F3;
	case VK_F4: return WINDOW_KEY_F4;
	case VK_F5: return WINDOW_KEY_F5;
	case VK_F6: return WINDOW_KEY_F6;
	case VK_F7: return WINDOW_KEY_F7;
	case VK_F8: return WINDOW_KEY_F8;
	case VK_F9: return WINDOW_KEY_F9;
	case VK_F10: return WINDOW_KEY_F10;
	case VK_F11: return WINDOW_KEY_F11;
	case VK_F12: return WINDOW_KEY_F12;
	
	case VK_LSHIFT: return WINDOW_KEY_SHIFT_L;
	case VK_RSHIFT: return WINDOW_KEY_SHIFT_R;
	case VK_LCONTROL: return WINDOW_KEY_CTRL_L;
	case VK_RCONTROL: return WINDOW_KEY_CTRL_R;
	case VK_LMENU: return WINDOW_KEY_ALT_L;
	case VK_RMENU: return WINDOW_KEY_ALT_R;
	
	case VK_SPACE: return WINDOW_KEY_SPACE;
	case VK_BACK: return WINDOW_KEY_BACKSPACE;
	case VK_TAB: return WINDOW_KEY_TAB;
	case VK_RETURN: return WINDOW_KEY_ENTER;
	case VK_ESCAPE: return WINDOW_KEY_ESCAPE;

	case VK_UP: return WINDOW_KEY_ARROW_UP;
	case VK_DOWN: return WINDOW_KEY_ARROW_DOWN;
	case VK_LEFT: return WINDOW_KEY_ARROW_LEFT;
	case VK_RIGHT: return WINDOW_KEY_ARROW_RIGHT;

	case VK_LBUTTON: return WINDOW_KEY_MOUSE_LEFT;
	case VK_MBUTTON: return WINDOW_KEY_MOUSE_MIDDLE;
	case VK_RBUTTON: return WINDOW_KEY_MOUSE_RIGHT;

	default: return -1;
	}

}

LRESULT CALLBACK window_WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_CREATE)
	{
		CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
	}

	struct window_data_windows* window_data = (struct window_data_windows*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (window_data != NULL)
	{
		struct window_event event;

		if (uMsg == WM_SIZE || uMsg == WM_MOVE) {
			RECT rect;
			if (GetClientRect(hwnd, &rect))
			{
				window_data->width = rect.right - rect.left;
				window_data->height = rect.bottom - rect.top;
				window_data->position_x = rect.left;
				window_data->position_y = rect.top;

				window_data->move_size = true;
			}
			return 0;
		}
		else if (uMsg == WM_CLOSE)
		{
			event.type = WINDOW_EVENT_DESTROY;
			_window_event_queue_add(window_data, &event);
			return 0;
		}
		else if (uMsg == WM_SETFOCUS)
		{
			event.type = WINDOW_EVENT_FOCUS;
			_window_event_queue_add(window_data, &event);
			return 0;
		}
		else if (uMsg == WM_KILLFOCUS)
		{
			event.type = WINDOW_EVENT_UNFOCUS;
			_window_event_queue_add(window_data, &event);
			return 0;
		}
		else if (uMsg == WM_MOUSEWHEEL)
		{
			event.type = WINDOW_EVENT_MOUSE_SCROLL;
			event.info.mouse_scroll.scroll_steps = GET_WHEEL_DELTA_WPARAM(wParam) / 120;
			_window_event_queue_add(window_data, &event);
			return 0;
		}
		else if (uMsg == WM_KEYDOWN || uMsg == WM_LBUTTONDOWN || uMsg == WM_MBUTTONDOWN || uMsg == WM_RBUTTONDOWN)
		{
			uint32_t key_code;

			if (uMsg == WM_KEYDOWN) key_code = wParam;
			else if (uMsg == WM_LBUTTONDOWN) key_code = VK_LBUTTON;
			else if (uMsg == WM_MBUTTONDOWN) key_code = VK_MBUTTON;
			else if (uMsg == WM_RBUTTONDOWN) key_code = VK_RBUTTON;

			int32_t key = _map_key(key_code);
			if (key != -1)
			{
				event.type = WINDOW_EVENT_KEY_DOWN;
				event.info.key_down.key = key;
				_window_event_queue_add(window_data, &event);
			}
			return 0;
		}
		else if (uMsg == WM_KEYUP || uMsg == WM_LBUTTONUP || uMsg == WM_MBUTTONUP || uMsg == WM_RBUTTONUP)
		{
			uint32_t key_code;

			if (uMsg == WM_KEYUP) key_code = wParam;
			else if (uMsg == WM_LBUTTONUP) key_code = VK_LBUTTON;
			else if (uMsg == WM_MBUTTONUP) key_code = VK_MBUTTON;
			else if (uMsg == WM_RBUTTONUP) key_code = VK_RBUTTON;

			int32_t key = _map_key(key_code);
			if (key != -1)
			{
				event.type = WINDOW_EVENT_KEY_UP;
				event.info.key_down.key = key;
				_window_event_queue_add(window_data, &event);
			}
			return 0;
		}
		else if (uMsg == WM_CHAR) {
			event.type = WINDOW_EVENT_CHARACTER;
			event.info.character.code_point = wParam;
			_window_event_queue_add(window_data, &event);
			return 0;
		}
	}

	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

bool window_init_context(void* transfered_context)
{
	if (transfered_context == NULL)
	{
		WNDCLASSW wc = {
			CS_HREDRAW | CS_VREDRAW | CS_CLASSDC,
			window_WinProc,
			0,
			0,
			GetModuleHandleW(NULL),
			NULL,
			LoadCursorW(NULL, IDC_ARROW),
			NULL,
			NULL,
			L"window_window_class"
		};

		if (RegisterClassW(&wc) == 0) return 1;
	}
	else
		context = transfered_context;

	return 0;
}

void window_deinit_context()
{
	if (context == &context_memory) UnregisterClassW(L"window_window_class", GetModuleHandleW(NULL));
}

void* window_get_context()
{
	return context;
}

void* window_create(int32_t posx, int32_t posy, uint32_t width, uint32_t height, uint8_t* name, bool visible)
{
	struct window_data_windows* window_data = malloc(sizeof(struct window_data_windows));
	if (window_data == NULL) return NULL;
	
	window_data->event_queue_length = _WINDOW_QUEUE_EXTENSION_EVENTS_COUNT;
	window_data->event_queue = malloc(_WINDOW_QUEUE_EXTENSION_EVENTS_COUNT * sizeof(struct window_event));
	if (window_data->event_queue == NULL)
	{
		free(window_data);
		return NULL;
	}
	window_data->free_event_queue_index = 0;
	window_data->last_event_queue_index = 0;

	uint32_t name_length = strlen(name) + 1;
	USHORT* wide_name = malloc(name_length * sizeof(USHORT));
	if (wide_name == NULL)
	{
		free(window_data->event_queue);
		free(window_data);
	}
	for (int32_t i = 0; i < name_length; i++) wide_name[i] = name[i];

	HWND hwnd = CreateWindowExW(
		0,
		L"window_window_class",
		wide_name,
		(visible ? WS_OVERLAPPEDWINDOW | WS_VISIBLE : 0),
		posx,
		posy,
		width + 16,
		height + 39,
		NULL,
		NULL,
		GetModuleHandleA(NULL),
		(LPVOID)window_data
	);

	free(wide_name);

	if (hwnd == NULL)
	{
		free(window_data->event_queue);
		free(window_data);
	}

	window_data->hwnd = hwnd;
	window_data->hdc = NULL;
	window_data->hglrc = NULL;

	RECT rect;
	GetClientRect(window_data->hwnd, &rect);

	window_data->width = rect.right - rect.left;
	window_data->height = rect.bottom - rect.top;
	window_data->position_x = rect.left;
	window_data->position_y = rect.top;
	window_data->move_size = false;

	window_data->selected = false;

	return window_data;
}

void window_destroy(void* window)
{
	struct window_data_windows* window_data = window;
	
	DestroyWindow(window_data->hwnd);

	free(window_data->event_queue);
	free(window_data);
}

void window_get_dimensions(void* window, uint32_t* width, uint32_t* height, int32_t* screen_position_x, int32_t* screen_position_y)
{
	struct window_data_windows* window_data = window;

	*width = window_data->width;
	*height = window_data->height;
	*screen_position_x = window_data->position_x;
	*screen_position_y = window_data->position_y;
}

bool window_is_selected(void* window)
{
	struct window_data_windows* window_data = window;
	return window_data->selected;
}

void window_get_mouse_cursor_position(void* window, int32_t* position_x, int32_t* position_y)
{

}

void window_set_mouse_cursor_position(void* window, int32_t x, int32_t y)
{

}

struct window_event* window_next_event(void* window)
{
	struct window_data_windows* window_data = window;

	while (true)
	{
		MSG message;

		if (window_data->last_event_queue_index != window_data->free_event_queue_index)
		{
			window_data->dispatched_event = window_data->event_queue[window_data->last_event_queue_index];
			window_data->last_event_queue_index = (window_data->last_event_queue_index + 1) % window_data->event_queue_length;

			return &window_data->dispatched_event;
		}
		else if (PeekMessageW(&message, window_data->hwnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessageW(&message);

			if (window_data->move_size)
			{
				window_data->move_size = false;

				window_data->dispatched_event.type = WINDOW_EVENT_MOVE_SIZE;
				window_data->dispatched_event.info.move_size.width = window_data->width;
				window_data->dispatched_event.info.move_size.height = window_data->height;
				window_data->dispatched_event.info.move_size.position_x = window_data->position_x;
				window_data->dispatched_event.info.move_size.position_y = window_data->position_y;

				return &window_data->dispatched_event;
			}
		}
		else return NULL;
	}
}

HWND window_windows_get_hwnd(void* window)
{
	struct window_data_windows* window_data = window;

	return window_data->hwnd;
}


//vulkan
bool window_vulkan_load()
{
	if ((context->vulkan.library = LoadLibraryA("vulkan-1.dll")) == NULL) return false;
	
	if ((context->vulkan.func.vkGetInstanceProcAddr = GetProcAddress(context->vulkan.library, "vkGetInstanceProcAddr")) == NULL)
	{
		FreeLibrary(context->vulkan.library);
		return false;
	}

	return true;
}

void window_vulkan_unload()
{
	FreeLibrary(context->vulkan.library);
}

PFN_vkGetInstanceProcAddr window_get_vkGetInstanceProcAddr()
{
	return context->vulkan.func.vkGetInstanceProcAddr;
}

VkResult window_vkCreateSurfaceKHR(void* window, VkInstance instance, VkSurfaceKHR* surface)
{
	struct window_data_windows* window_data = window;

	PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR_func = (PFN_vkCreateWin32SurfaceKHR)context->vulkan.func.vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
	if (vkCreateWin32SurfaceKHR_func == NULL) return VK_ERROR_INITIALIZATION_FAILED;

	VkWin32SurfaceCreateInfoKHR create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	create_info.hwnd = window_data->hwnd;
	create_info.hinstance = GetModuleHandleW(NULL);

	return vkCreateWin32SurfaceKHR_func(instance, &create_info, NULL, surface);
}

uint8_t* window_get_VK_KHR_PLATFORM_SURFACE_EXTENSION_NAME()
{
	return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
}

//opengl

bool window_opengl_load()
{
	if ((context->opengl.library = LoadLibraryA("opengl32.dll")) == NULL) return false;

	context->opengl.func.wglCreateContext = GetProcAddress(context->opengl.library, "wglCreateContext");
	context->opengl.func.wglDeleteContext = GetProcAddress(context->opengl.library, "wglDeleteContext");
	context->opengl.func.wglMakeCurrent = GetProcAddress(context->opengl.library, "wglMakeCurrent");
	context->opengl.func.wglGetProcAddress = GetProcAddress(context->opengl.library, "wglGetProcAddress");

	if (
		context->opengl.func.wglCreateContext == NULL ||
		context->opengl.func.wglDeleteContext == NULL ||
		context->opengl.func.wglMakeCurrent == NULL ||
		context->opengl.func.wglGetProcAddress == NULL
	)
	{
		FreeLibrary(context->opengl.library);
		return false;
	}

	return true;
}

void window_opengl_unload()
{
	FreeLibrary(context->opengl.library);
}

bool window_glCreateContext(void* window, int32_t version_major, int32_t version_minor, void* share_window)
{
	struct window_data_windows* window_data = window;
	struct window_data_windows* share_window_data = share_window;

	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;

	HWND dummy_hwnd;
	HDC dummy_hdc;
	HGLRC dummy_hglrc;

	bool result = true;

	bool
		created_dummy_hwnd = false,
		got_dummy_hdc = false,
		created_dummy_hglrc = false,
		got_hdc = false,
		created_hglrc = false
	;

	if ((dummy_hwnd = CreateWindowExW(
		0,
		L"window_window_class",
		L"dummy_window",
		NULL,
		0,
		0,
		0 + 16,
		0 + 39,
		NULL,
		NULL,
		GetModuleHandleA(NULL),
		0
	)) == NULL) result = false;
	else created_dummy_hwnd = true;

	if (result == true)
	{
		if ((dummy_hdc = GetDC(dummy_hwnd)) == NULL) result = false;
		else got_dummy_hdc = true;
	}

	PIXELFORMATDESCRIPTOR dummy_pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		32,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24,
		8,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	int dummy_pixelFormat;
	if (result == true) if ((dummy_pixelFormat = ChoosePixelFormat(dummy_hdc, &dummy_pfd)) == 0) result = false;
	if (result == true) if (SetPixelFormat(dummy_hdc, dummy_pixelFormat, &dummy_pfd) == FALSE) result = false;
	if (result == true)
	{
		if ((dummy_hglrc = context->opengl.func.wglCreateContext(dummy_hdc)) == NULL) result = false;
		else created_dummy_hglrc = true;
	}
	if (result == true) if (context->opengl.func.wglMakeCurrent(dummy_hdc, dummy_hglrc) == FALSE) result = false;

	if (result == true) if (
		(wglCreateContextAttribsARB = context->opengl.func.wglGetProcAddress("wglCreateContextAttribsARB")) == NULL ||
		(wglChoosePixelFormatARB = context->opengl.func.wglGetProcAddress("wglChoosePixelFormatARB")) == NULL
	) result = false;

	if (result == true)
	{
		if ((window_data->hdc = GetDC(window_data->hwnd)) == NULL) result = false;
		else got_hdc = true;
	}

	int pixel_format_attribs[] =
	{
		WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
		WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
		WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB,         32,
		WGL_DEPTH_BITS_ARB,         24,
		WGL_STENCIL_BITS_ARB,       8,
		0
	};

	int pixel_format;
	UINT num_formats;
	if (result == true) if (wglChoosePixelFormatARB(window_data->hdc, pixel_format_attribs, 0, 1, &pixel_format, &num_formats) == FALSE) result = false;

	PIXELFORMATDESCRIPTOR pfd;
	if (result == true) if (DescribePixelFormat(window_data->hdc, pixel_format, sizeof(pfd), &pfd) == 0) result = false;
	if (result == true) if (SetPixelFormat(window_data->hdc, pixel_format, &pfd) == FALSE) result = false;

	const int attribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, version_major,
		WGL_CONTEXT_MINOR_VERSION_ARB, version_minor,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};

	if (result == true)
	{
		if ((window_data->hglrc = wglCreateContextAttribsARB(window_data->hdc, share_window_data ? share_window_data->hglrc : 0, attribs)) == NULL) result = false;
		else created_hglrc = true;
	}

	if (result == true) if (context->opengl.func.wglMakeCurrent(window_data->hdc, window_data->hglrc) == FALSE) result = false;

	if (result == true) if ((window_data->wglSwapIntervalEXT = context->opengl.func.wglGetProcAddress("wglSwapIntervalEXT")) == NULL) result = false;

	context->opengl.func.wglMakeCurrent(NULL, NULL);

	if (result == false && created_hglrc == true) context->opengl.func.wglDeleteContext(window_data->hglrc);
	if (result == false && got_hdc == true) ReleaseDC(window_data->hwnd, window_data->hdc);
	
	if (created_dummy_hglrc == true) context->opengl.func.wglDeleteContext(dummy_hglrc);
	if (got_dummy_hdc == true) ReleaseDC(dummy_hwnd, dummy_hdc);
	if (created_dummy_hwnd == true) DestroyWindow(dummy_hwnd);

	return result;
}

void window_glDeleteContext(void* window)
{
	struct window_data_windows* window_data = window;

	context->opengl.func.wglMakeCurrent(NULL, NULL);
	context->opengl.func.wglDeleteContext(window_data->hglrc);
	ReleaseDC(window_data->hwnd, window_data->hdc);
}

bool window_glMakeCurrent(void* window)
{
	struct window_data_windows* window_data = window;

	bool result = true;

	if (window_data == NULL) context->opengl.func.wglMakeCurrent(NULL, NULL);
	else if ((result = (context->opengl.func.wglMakeCurrent(window_data->hdc, window_data->hglrc) == TRUE)) == true) context->opengl.current_window = window;
	return result;
}

bool window_glSwapInterval(int interval)
{
	return (context->opengl.current_window->wglSwapIntervalEXT(interval) == TRUE);
}

void (*window_glGetProcAddress(uint8_t* name)) (void)
{
	void (*function)(void);

	if ((function = context->opengl.func.wglGetProcAddress(name)) != NULL) return function;
	else return GetProcAddress(context->opengl.library, name);
}

bool window_glSwapBuffers(void* window)
{
	return (SwapBuffers(((struct window_data_windows*)(window))->hdc) == TRUE);
}