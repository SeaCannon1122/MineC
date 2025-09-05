#include <cwindow/cwindow.h>
#include <GL/wgl.h>

#include <stdio.h>
#include <stdlib.h>

#define WINDOW_QUEUE_EXTENSION_EVENTS_COUNT 256

struct cwindow_context
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
	} opengl;

	USHORT name[];
};

struct cwindow
{
	cwindow_context* context;

	HWND hwnd;
	HICON icon;

	HDC hdc;
	HGLRC hglrc;
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

	struct cwindow_event dispatched_event;
	bool move_size;

	CRITICAL_SECTION dimension_cr;
	uint32_t width;
	uint32_t height;
	uint32_t position_x;
	uint32_t position_y;

	bool selected;

	struct cwindow_event* event_queue;
	uint32_t event_queue_length;
	uint32_t free_event_queue_index;
	uint32_t last_event_queue_index;

	bool on_get_next_event;

	struct
	{
		CRITICAL_SECTION in_cr;
		bool freeze;
		bool frozen;
		CRITICAL_SECTION freeze_cr;
	} freeze_queue_state;

	USHORT name[];
};

void _cwindow_event_queue_add(struct cwindow* __restrict window, struct cwindow_event* __restrict event)
{
	if ((window->free_event_queue_index + 1) % window->event_queue_length == window->last_event_queue_index)
	{
		void* new_event_queue = realloc(window->event_queue, (window->event_queue_length + WINDOW_QUEUE_EXTENSION_EVENTS_COUNT) * sizeof(struct cwindow_event));
		if (new_event_queue == NULL)
		{
			window->event_queue[window->last_event_queue_index].type = CWINDOW_EVENT_DESTROY;
			return;
		}

		window->event_queue = new_event_queue;

		if (window->free_event_queue_index != window->event_queue_length - 1)
		{
			memmove(
				&window->event_queue[window->last_event_queue_index + WINDOW_QUEUE_EXTENSION_EVENTS_COUNT],
				&window->event_queue[window->last_event_queue_index],
				(window->event_queue_length - window->last_event_queue_index) * sizeof(struct cwindow_event)
			);
			window->last_event_queue_index += WINDOW_QUEUE_EXTENSION_EVENTS_COUNT;
		}

		window->event_queue_length += WINDOW_QUEUE_EXTENSION_EVENTS_COUNT;
	}

	window->event_queue[window->free_event_queue_index] = *event;
	window->free_event_queue_index = (window->free_event_queue_index + 1) % window->event_queue_length;
}

int32_t _map_key(int32_t key) 
{

	switch (key) {

	case '1': return CWINDOW_KEY_1;
	case '2': return CWINDOW_KEY_2;
	case '3': return CWINDOW_KEY_3;
	case '4': return CWINDOW_KEY_4;
	case '5': return CWINDOW_KEY_5;
	case '6': return CWINDOW_KEY_6;
	case '7': return CWINDOW_KEY_7;
	case '8': return CWINDOW_KEY_8;
	case '9': return CWINDOW_KEY_9;
	case '0': return CWINDOW_KEY_0;

	case 'A': return CWINDOW_KEY_A;
	case 'B': return CWINDOW_KEY_B;
	case 'C': return CWINDOW_KEY_C;
	case 'D': return CWINDOW_KEY_D;
	case 'E': return CWINDOW_KEY_E;
	case 'F': return CWINDOW_KEY_F;
	case 'G': return CWINDOW_KEY_G;
	case 'H': return CWINDOW_KEY_H;
	case 'I': return CWINDOW_KEY_I;
	case 'J': return CWINDOW_KEY_J;
	case 'K': return CWINDOW_KEY_K;
	case 'L': return CWINDOW_KEY_L;
	case 'M': return CWINDOW_KEY_M;
	case 'N': return CWINDOW_KEY_N;
	case 'O': return CWINDOW_KEY_O;
	case 'P': return CWINDOW_KEY_P;
	case 'Q': return CWINDOW_KEY_Q;
	case 'R': return CWINDOW_KEY_R;
	case 'S': return CWINDOW_KEY_S;
	case 'T': return CWINDOW_KEY_T;
	case 'U': return CWINDOW_KEY_U;
	case 'V': return CWINDOW_KEY_V;
	case 'W': return CWINDOW_KEY_W;
	case 'X': return CWINDOW_KEY_X;
	case 'Y': return CWINDOW_KEY_Y;
	case 'Z': return CWINDOW_KEY_Z;

	case VK_ADD: return CWINDOW_KEY_PLUS;
	case VK_SUBTRACT: return CWINDOW_KEY_MINUS;

	case VK_F1: return CWINDOW_KEY_F1;
	case VK_F2: return CWINDOW_KEY_F2;
	case VK_F3: return CWINDOW_KEY_F3;
	case VK_F4: return CWINDOW_KEY_F4;
	case VK_F5: return CWINDOW_KEY_F5;
	case VK_F6: return CWINDOW_KEY_F6;
	case VK_F7: return CWINDOW_KEY_F7;
	case VK_F8: return CWINDOW_KEY_F8;
	case VK_F9: return CWINDOW_KEY_F9;
	case VK_F10: return CWINDOW_KEY_F10;
	case VK_F11: return CWINDOW_KEY_F11;
	case VK_F12: return CWINDOW_KEY_F12;
	
	case VK_LSHIFT: return CWINDOW_KEY_SHIFT_L;
	case VK_RSHIFT: return CWINDOW_KEY_SHIFT_R;
	case VK_LCONTROL: return CWINDOW_KEY_CTRL_L;
	case VK_RCONTROL: return CWINDOW_KEY_CTRL_R;
	case VK_LMENU: return CWINDOW_KEY_ALT_L;
	case VK_RMENU: return CWINDOW_KEY_ALT_R;
	
	case VK_SPACE: return CWINDOW_KEY_SPACE;
	case VK_BACK: return CWINDOW_KEY_BACKSPACE;
	case VK_TAB: return CWINDOW_KEY_TAB;
	case VK_RETURN: return CWINDOW_KEY_ENTER;
	case VK_ESCAPE: return CWINDOW_KEY_ESCAPE;

	case VK_UP: return CWINDOW_KEY_ARROW_UP;
	case VK_DOWN: return CWINDOW_KEY_ARROW_DOWN;
	case VK_LEFT: return CWINDOW_KEY_ARROW_LEFT;
	case VK_RIGHT: return CWINDOW_KEY_ARROW_RIGHT;

	case VK_LBUTTON: return CWINDOW_KEY_MOUSE_LEFT;
	case VK_MBUTTON: return CWINDOW_KEY_MOUSE_MIDDLE;
	case VK_RBUTTON: return CWINDOW_KEY_MOUSE_RIGHT;

	default: return -1;
	}

}

LRESULT CALLBACK cwindow_WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	bool result_set = false;
	LRESULT result = 0;

	if (uMsg == WM_CREATE)
	{
		CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
	}

	cwindow* window = (cwindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (window != NULL) if (window->hwnd != NULL)
	{
		cwindow_event event;

		if (uMsg == WM_SIZE || uMsg == WM_MOVE) {
			RECT client_rect, window_rect;
			GetClientRect(hwnd, &client_rect);
			GetWindowRect(hwnd, &window_rect);

			EnterCriticalSection(&window->dimension_cr);
			window->width = client_rect.right;
			window->height = client_rect.bottom;
			window->position_x = window_rect.left;
			window->position_y = window_rect.top;
			LeaveCriticalSection(&window->dimension_cr);

			window->move_size = true;
			result_set = true;
		}
		else if (uMsg == WM_CLOSE)
		{
			event.type = CWINDOW_EVENT_DESTROY;
			_cwindow_event_queue_add(window, &event);
			result_set = true;
		}
		else if (uMsg == WM_SETFOCUS)
		{
			event.type = CWINDOW_EVENT_FOCUS;
			_cwindow_event_queue_add(window, &event);
			result_set = true;
		}
		else if (uMsg == WM_KILLFOCUS)
		{
			event.type = CWINDOW_EVENT_UNFOCUS;
			_cwindow_event_queue_add(window, &event);
			result_set = true;
		}
		else if (uMsg == WM_MOUSEWHEEL)
		{
			event.type = CWINDOW_EVENT_MOUSE_SCROLL;
			event.info.mouse_scroll.scroll_steps = GET_WHEEL_DELTA_WPARAM(wParam) / 120;
			_cwindow_event_queue_add(window, &event);
			result_set = true;
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
				event.type = CWINDOW_EVENT_KEY_DOWN;
				event.info.key_down.key = key;
				_cwindow_event_queue_add(window, &event);
			}
			result_set = true;
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
				event.type = CWINDOW_EVENT_KEY_UP;
				event.info.key_down.key = key;
				_cwindow_event_queue_add(window, &event);
			}
			result_set = true;
		}
		else if (uMsg == WM_CHAR) {
			event.type = CWINDOW_EVENT_CHARACTER;
			event.info.character.code_point = wParam;
			_cwindow_event_queue_add(window, &event);
			result_set = true;
		}
	
		if (window->on_get_next_event)
		{
			LeaveCriticalSection(&window->freeze_queue_state.in_cr);

			EnterCriticalSection(&window->freeze_queue_state.freeze_cr);
			bool freeze = window->freeze_queue_state.freeze;
			window->freeze_queue_state.freeze = false;
			LeaveCriticalSection(&window->freeze_queue_state.freeze_cr);

			if (freeze)
			{
				bool frozen = false;
				while (frozen == false)
				{
					EnterCriticalSection(&window->freeze_queue_state.freeze_cr);
					frozen = window->freeze_queue_state.frozen;
					window->freeze_queue_state.frozen = false;
					LeaveCriticalSection(&window->freeze_queue_state.freeze_cr);
				}
			}

			EnterCriticalSection(&window->freeze_queue_state.in_cr);
		}
	}

	if (result_set == false) result = DefWindowProcW(hwnd, uMsg, wParam, lParam);
	return result;
}

cwindow_context* cwindow_context_create(const uint8_t* name)
{
	uint32_t name_length = strlen(name) + 1;

	cwindow_context* context = malloc(sizeof(cwindow_context) + name_length * sizeof(USHORT));
	if (context == NULL) return NULL;

	for (uint32_t i = 0; i < name_length; i++) context->name[i] = name[i];

	WNDCLASSW wc =
	{
		CS_HREDRAW | CS_VREDRAW | CS_CLASSDC,
		cwindow_WinProc,
		0,
		0,
		GetModuleHandleW(NULL),
		NULL,
		LoadCursorW(NULL, (LPCWSTR)IDC_ARROW),
		NULL,
		NULL,
		context->name
	};

	if (RegisterClassW(&wc) == 0)
	{
		free(context);
		return NULL;
	}

	return context;
}

void cwindow_context_destroy(cwindow_context* context)
{
	UnregisterClassW(context->name, GetModuleHandleW(NULL));
	free(context);
}

void cwindow_context_get_display_dimensions(cwindow_context* __restrict context, uint32_t* __restrict width, uint32_t* __restrict height)
{

}

cwindow* cwindow_create(cwindow_context* __restrict context, int32_t posx, int32_t posy, uint32_t width, uint32_t height, const uint8_t* __restrict name, bool visible)
{
	uint32_t name_length = strlen(name) + 1;

	cwindow* window = malloc(sizeof(cwindow) + name_length * sizeof(USHORT));
	if (window == NULL) return NULL;

	for (int32_t i = 0; i < name_length; i++) window->name[i] = name[i];
	
	if ((window->event_queue = malloc(WINDOW_QUEUE_EXTENSION_EVENTS_COUNT * sizeof(cwindow_event))) == NULL)
	{
		free(window);
		return NULL;
	}
	window->event_queue_length = WINDOW_QUEUE_EXTENSION_EVENTS_COUNT;
	window->last_event_queue_index = 0;
	window->free_event_queue_index = 0;

	InitializeCriticalSection(&window->dimension_cr);
	InitializeCriticalSection(&window->freeze_queue_state.in_cr);
	InitializeCriticalSection(&window->freeze_queue_state.freeze_cr);

	window->hwnd = NULL;

	if ((window->hwnd = CreateWindowExW(
		0,
		context->name,
		window->name,
		(visible ? WS_OVERLAPPEDWINDOW | WS_VISIBLE : 0),
		posx,
		posy,
		width + 16,
		height + 39,
		NULL,
		NULL,
		GetModuleHandleA(NULL),
		(LPVOID)window
	)) == NULL)
	{
		DeleteCriticalSection(&window->dimension_cr);
		DeleteCriticalSection(&window->freeze_queue_state.in_cr);
		DeleteCriticalSection(&window->freeze_queue_state.freeze_cr);
		free(window->event_queue);
		free(window);
	}

	window->context = context;

	RECT client_rect, window_rect;
	GetClientRect(window->hwnd, &client_rect);
	GetWindowRect(window->hwnd, &window_rect);

	window->width = client_rect.right;
	window->height = client_rect.bottom;
	window->position_x = window_rect.left;
	window->position_y = window_rect.top;
	
	window->move_size = false;

	window->selected = false;

	window->freeze_queue_state.freeze = false;
	window->freeze_queue_state.frozen = false;
	window->on_get_next_event = false;

	return window;
}

void cwindow_destroy(cwindow* window)
{
	DestroyWindow(window->hwnd);

	DeleteCriticalSection(&window->freeze_queue_state.freeze_cr);
	DeleteCriticalSection(&window->freeze_queue_state.in_cr);
	DeleteCriticalSection(&window->dimension_cr);

	if (window->icon) DestroyIcon(window->icon);

	free(window->event_queue);
	free(window);
}

bool cwindow_set_icon(cwindow* __restrict window, const uint32_t* __restrict icon_rgba_pixel_data, uint32_t icon_width, uint32_t icon_height)
{
	if (window->icon) DestroyIcon(window->icon);
	window->icon = NULL;

	BITMAPV5HEADER bi = { 0 };
	bi.bV5Size = sizeof(BITMAPV5HEADER);
	bi.bV5Width = icon_width;
	bi.bV5Height = -icon_height;
	bi.bV5Planes = 1;
	bi.bV5BitCount = 32;
	bi.bV5Compression = BI_BITFIELDS;
	bi.bV5RedMask = 0x00FF0000;
	bi.bV5GreenMask = 0x0000FF00;
	bi.bV5BlueMask = 0x000000FF;
	bi.bV5AlphaMask = 0xFF000000;

	HDC hdc = GetDC(NULL);
	if (hdc == NULL) return false;

	uint8_t* bits = NULL;
	HBITMAP dib_section = CreateDIBSection(hdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, &bits, NULL, 0);

	ReleaseDC(NULL, hdc);

	if (dib_section == NULL || bits == NULL) return false;

	uint8_t* rgba_pixels = (uint8_t*)icon_rgba_pixel_data;

	for (uint32_t i = 0; i < icon_width * icon_height; i++) 
	{
		bits[4 * i + 0] = rgba_pixels[i * 4 + 3] * rgba_pixels[i * 4 + 2] / 255;
		bits[4 * i + 1] = rgba_pixels[i * 4 + 3] * rgba_pixels[i * 4 + 1] / 255;
		bits[4 * i + 2] = rgba_pixels[i * 4 + 3] * rgba_pixels[i * 4 + 0] / 255;
		bits[4 * i + 3] = rgba_pixels[i * 4 + 3];
	}

	HBITMAP bitmap = CreateBitmap(icon_width, icon_height, 1, 1, NULL);
	if (bitmap == NULL) {
		DeleteObject(dib_section);
		return false;
	}

	ICONINFO icon_info = { 0 };
	icon_info.fIcon = TRUE;
	icon_info.xHotspot = 0;
	icon_info.yHotspot = 0;
	icon_info.hbmMask = bitmap;
	icon_info.hbmColor = dib_section;

	window->icon = CreateIconIndirect(&icon_info);

	DeleteObject(bitmap);
	DeleteObject(dib_section);

	if (window->icon == NULL) return false;

	SendMessage(window->hwnd, WM_SETICON, ICON_SMALL, (LPARAM)window->icon);
	SendMessage(window->hwnd, WM_SETICON, ICON_BIG, (LPARAM)window->icon);

	return true;
}

void cwindow_get_dimensions(cwindow* __restrict window, uint32_t* __restrict width, uint32_t* __restrict height, int32_t* __restrict posx, int32_t* __restrict posy)
{
	EnterCriticalSection(&window->dimension_cr);
	*width = window->width;
	*height = window->height;
	*posx = window->position_x;
	*posy = window->position_y;
	LeaveCriticalSection(&window->dimension_cr);
}

bool cwindow_is_selected(cwindow* window)
{
	return window->selected;
}

void cwindow_get_mouse_cursor_position(cwindow* __restrict window, int32_t* __restrict posx, int32_t* __restrict posy)
{

}

void cwindow_set_mouse_cursor_position(cwindow* window, int32_t posx, int32_t posy)
{

}

const cwindow_event* cwindow_next_event(cwindow* window)
{
	EnterCriticalSection(&window->freeze_queue_state.freeze_cr);
	bool freeze = window->freeze_queue_state.freeze;
	window->freeze_queue_state.freeze = false;
	LeaveCriticalSection(&window->freeze_queue_state.freeze_cr);

	if (freeze)
	{
		bool frozen = false;
		while (frozen == false)
		{
			EnterCriticalSection(&window->freeze_queue_state.freeze_cr);
			frozen = window->freeze_queue_state.frozen;
			window->freeze_queue_state.frozen = false;
			LeaveCriticalSection(&window->freeze_queue_state.freeze_cr);
		}
	}

	window->on_get_next_event = true;
	EnterCriticalSection(&window->freeze_queue_state.in_cr);

	const cwindow_event* event = NULL;

	while (true)
	{
		MSG message;

		if (window->last_event_queue_index != window->free_event_queue_index)
		{
			window->dispatched_event = window->event_queue[window->last_event_queue_index];
			window->last_event_queue_index = (window->last_event_queue_index + 1) % window->event_queue_length;

			event = &window->dispatched_event;
			break;
		}
		else if (PeekMessageW(&message, window->hwnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessageW(&message);

			if (window->move_size)
			{
				window->move_size = false;

				window->dispatched_event.type = CWINDOW_EVENT_MOVE_SIZE;
				window->dispatched_event.info.move_size.width = window->width;
				window->dispatched_event.info.move_size.height = window->height;
				window->dispatched_event.info.move_size.position_x = window->position_x;
				window->dispatched_event.info.move_size.position_y = window->position_y;

				event = &window->dispatched_event;
				break;
			}
		}
		else break;
	}

	LeaveCriticalSection(&window->freeze_queue_state.in_cr);
	window->on_get_next_event = false;

	return event;
}

void cwindow_freeze_event_queue(cwindow* window)
{
	EnterCriticalSection(&window->freeze_queue_state.freeze_cr);
	window->freeze_queue_state.freeze = true;
	LeaveCriticalSection(&window->freeze_queue_state.freeze_cr);

	EnterCriticalSection(&window->freeze_queue_state.in_cr);

	EnterCriticalSection(&window->freeze_queue_state.freeze_cr);
	window->freeze_queue_state.frozen = true;
	LeaveCriticalSection(&window->freeze_queue_state.freeze_cr);
}

void cwindow_unfreeze_event_queue(cwindow* window)
{
	LeaveCriticalSection(&window->freeze_queue_state.in_cr);
}

HWND cwindow_impl_windows_get_hwnd(cwindow* window)
{
	return window->hwnd;
}

//vulkan
bool cwindow_context_graphics_vulkan_load(cwindow_context* __restrict context, PFN_vkGetInstanceProcAddr* __restrict gpa)
{
	if ((context->vulkan.library = LoadLibraryA("vulkan-1.dll")) == NULL) return false;
	
	if ((context->vulkan.func.vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetProcAddress(context->vulkan.library, "vkGetInstanceProcAddr")) == NULL)
	{
		FreeLibrary(context->vulkan.library);
		return false;
	}

	*gpa = context->vulkan.func.vkGetInstanceProcAddr;
	return true;
}

void cwindow_context_graphics_vulkan_unload(cwindow_context* context)
{
	FreeLibrary(context->vulkan.library);
}

VkResult cwindow_vkCreateSurfaceKHR(cwindow* __restrict window, VkInstance __restrict instance, VkSurfaceKHR* __restrict surface)
{
	PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR_func = (PFN_vkCreateWin32SurfaceKHR)window->context->vulkan.func.vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
	if (vkCreateWin32SurfaceKHR_func == NULL) return VK_ERROR_INITIALIZATION_FAILED;

	VkWin32SurfaceCreateInfoKHR create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	create_info.hwnd = window->hwnd;
	create_info.hinstance = GetModuleHandleW(NULL);

	return vkCreateWin32SurfaceKHR_func(instance, &create_info, NULL, surface);
}

//opengl

bool cwindow_context_graphics_opengl_load(cwindow_context* context)
{
	if ((context->opengl.library = LoadLibraryA("opengl32.dll")) == NULL) return false;

	context->opengl.func.wglCreateContext = (PFNWGLCREATECONTEXTPROC)GetProcAddress(context->opengl.library, "wglCreateContext");
	context->opengl.func.wglDeleteContext = (PFNWGLDELETECONTEXTPROC)GetProcAddress(context->opengl.library, "wglDeleteContext");
	context->opengl.func.wglMakeCurrent = (PFNWGLMAKECURRENTPROC)GetProcAddress(context->opengl.library, "wglMakeCurrent");
	context->opengl.func.wglGetProcAddress = (PFNWGLGETPROCADDRESSPROC)GetProcAddress(context->opengl.library, "wglGetProcAddress");

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

void cwindow_context_graphics_opengl_unload(cwindow_context* context)
{
	FreeLibrary(context->opengl.library);
}

bool cwindow_glCreateContext(cwindow* __restrict window, int32_t version_major, int32_t version_minor, cwindow* __restrict share_window, bool* __restrict glSwapIntervalEXT_support)
{
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
		window->context->name,
		L"dummy_window",
		0,
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
		if ((dummy_hglrc = window->context->opengl.func.wglCreateContext(dummy_hdc)) == NULL) result = false;
		else created_dummy_hglrc = true;
	}
	if (result == true) if (window->context->opengl.func.wglMakeCurrent(dummy_hdc, dummy_hglrc) == FALSE) result = false;

	if (result == true) if (
		(wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)window->context->opengl.func.wglGetProcAddress("wglCreateContextAttribsARB")) == NULL ||
		(wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)window->context->opengl.func.wglGetProcAddress("wglChoosePixelFormatARB")) == NULL
	) result = false;

	if (result == true)
	{
		if ((window->hdc = GetDC(window->hwnd)) == NULL) result = false;
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
	if (result == true) if (wglChoosePixelFormatARB(window->hdc, pixel_format_attribs, 0, 1, &pixel_format, &num_formats) == FALSE) result = false;

	PIXELFORMATDESCRIPTOR pfd;
	if (result == true) if (DescribePixelFormat(window->hdc, pixel_format, sizeof(pfd), &pfd) == 0) result = false;
	if (result == true) if (SetPixelFormat(window->hdc, pixel_format, &pfd) == FALSE) result = false;

	const int attribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, version_major,
		WGL_CONTEXT_MINOR_VERSION_ARB, version_minor,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB,
		WGL_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB, WGL_LOSE_CONTEXT_ON_RESET_ARB,
		0
	};

	if (result == true)
	{
		if ((window->hglrc = wglCreateContextAttribsARB(window->hdc, share_window ? share_window->hglrc : 0, attribs)) == NULL) result = false;
		else created_hglrc = true;
	}

	if (result == true) if (window->context->opengl.func.wglMakeCurrent(window->hdc, window->hglrc) == FALSE) result = false;

	if (result == true) 
	{
		if ((window->wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)window->context->opengl.func.wglGetProcAddress("wglSwapIntervalEXT")) != NULL) *glSwapIntervalEXT_support = true;
		else *glSwapIntervalEXT_support = false;
	}

	window->context->opengl.func.wglMakeCurrent(NULL, NULL);

	if (result == false && created_hglrc == true) window->context->opengl.func.wglDeleteContext(window->hglrc);
	if (result == false && got_hdc == true) ReleaseDC(window->hwnd, window->hdc);
	
	if (created_dummy_hglrc == true) window->context->opengl.func.wglDeleteContext(dummy_hglrc);
	if (got_dummy_hdc == true) ReleaseDC(dummy_hwnd, dummy_hdc);
	if (created_dummy_hwnd == true) DestroyWindow(dummy_hwnd);

	return result;
}

void cwindow_glDestroyContext(cwindow* window)
{
	window->context->opengl.func.wglDeleteContext(window->hglrc);
	ReleaseDC(window->hwnd, window->hdc);
}

bool cwindow_glMakeCurrent(cwindow* window, bool current)
{
	if (current) return (window->context->opengl.func.wglMakeCurrent(window->hdc, window->hglrc) == TRUE);

	window->context->opengl.func.wglMakeCurrent(NULL, NULL);
	return true;
}

bool cwindow_glSwapIntervalEXT(cwindow* window, int interval)
{
	 return (window->wglSwapIntervalEXT(interval) == TRUE);
}

void (*cwindow_glGetProcAddress(cwindow* __restrict window, const uint8_t* __restrict name)) (void)
{
	void (*function)(void);

	if ((function = (void (*)(void))window->context->opengl.func.wglGetProcAddress(name)) != NULL) return function;
	else return (void (*)(void))GetProcAddress(window->context->opengl.library, name);
}

bool cwindow_glSwapBuffers(cwindow* window)
{
	return (SwapBuffers(window->hdc) == TRUE);
}