#include "platform.h"

#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

struct window_state {
	HWND hwnd;

	int32_t window_width;
	int32_t window_height;
	int32_t position_x;
	int32_t position_y;
};

struct window_event_queue {

	uint32_t event_destroy;

	uint32_t event_focus;

	uint32_t event_unfocus;

	uint32_t event_mouse_scroll;
	uint32_t scroll_steps;

	uint32_t event_char;
	uint32_t unicode;

	uint32_t event_key_down;
	uint32_t event_key_up;
	uint32_t key;

	uint32_t event_move_size;
};

WNDCLASSW wc;
HWND console;

struct window_state window_states[MAX_WINDOW_COUNT] = { 0 };
struct window_event_queue window_event_queues[MAX_WINDOW_COUNT] = { 0 };

void* dynamic_library_load(uint8_t* src) {

	uint32_t src_length = 0;
	for (; src[src_length] != 0; src_length++);

	uint8_t* src_platform = _malloca(src_length + sizeof(".dll"));

	for (uint32_t i = 0; i < src_length; i++) src_platform[i] = src[i];
	src_platform[src_length] = '.';
	src_platform[src_length + 1] = 'd';
	src_platform[src_length + 2] = 'l';
	src_platform[src_length + 3] = 'l';
	src_platform[src_length + 4] = '\0';

	return LoadLibraryA(src_platform);

}

void (*dynamic_library_get_function(void* library_handle, uint8_t* function_name)) (void) {
	return GetProcAddress(library_handle, function_name);
}

void dynamic_library_unload(void* library_handle) {
	FreeLibrary(library_handle);
}

void set_console_cursor_position(int32_t x, int32_t y) {
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD) { (SHORT)x, (SHORT)y });
}

void sleep_for_ms(uint32_t time_in_milliseconds) {
	Sleep(time_in_milliseconds);
}

double get_time() {
	LARGE_INTEGER frequency, start;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&start);

	return (double)1000 * ((double)start.QuadPart / (double)frequency.QuadPart);
}

void* create_thread(void (address)(void*), void* args) {
	return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)address, args, 0, NULL);
}

void join_thread(void* thread_handle) {
	WaitForSingleObject(thread_handle, INFINITE);
	CloseHandle(thread_handle);
}

uint32_t get_screen_width() {
	return GetSystemMetrics(SM_CXSCREEN);
}

uint32_t get_screen_height() {
	return GetSystemMetrics(SM_CYSCREEN);
}

//window functions

void show_console_window() {
	ShowWindow(console, SW_SHOW);;
}

void hide_console_window() {
	ShowWindow(console, SW_HIDE);
}

uint32_t window_create(uint32_t posx, uint32_t posy, uint32_t width, uint32_t height, uint8_t* name, uint32_t visible) {

	uint32_t next_free_window_index = 0;
	for (; next_free_window_index < MAX_WINDOW_COUNT; next_free_window_index++) if (window_states[next_free_window_index].hwnd == NULL) break;
	if (next_free_window_index == MAX_WINDOW_COUNT) return WINDOW_CREATION_FAILED;

	int32_t name_length = 1;
	for (; name[name_length - 1] != '\0'; name_length++);

	USHORT* wide_name = _malloca(name_length * sizeof(USHORT));

	for (int32_t i = 0; i < name_length; i++) wide_name[i] = (*(USHORT*)&name[i] & 255);

	window_states[next_free_window_index].hwnd = CreateWindowExW(
		0,
		wc.lpszClassName,
		wide_name,
		(visible ? WS_OVERLAPPEDWINDOW | WS_VISIBLE : 0),
		posx,
		posy,
		width + 16,
		height + 39,
		NULL,
		NULL,
		GetModuleHandleA(NULL),
		NULL
	);

	if(window_states[next_free_window_index].hwnd == NULL) return WINDOW_CREATION_FAILED;

	window_states[next_free_window_index].window_width = width;
	window_states[next_free_window_index].window_height = height;
	window_states[next_free_window_index].position_x = posx;
	window_states[next_free_window_index].position_y = posy;

	return next_free_window_index;
}

uint32_t window_get_width(uint32_t window) {
	return window_states[window].window_width;
}

uint32_t window_get_height(uint32_t window) {
	return window_states[window].window_height;
}

int32_t window_is_selected(uint32_t window) {
	return window_states[window].hwnd == GetForegroundWindow();
}

void window_destroy(uint32_t window) {
	DestroyWindow(window_states[window].hwnd);
	window_states[window].hwnd = NULL;
}

struct point2d_int window_get_mouse_cursor_position(uint32_t window) {
	
	POINT position;
	GetCursorPos(&position);
	RECT window_rect;
	GetWindowRect(window_states[window].hwnd, &window_rect);

	struct point2d_int pos = { position.x - window_rect.left - 8, position.y - window_rect.top - 31 };
	if (pos.x > 0 && pos.x < window_states[window].window_width - 1 && pos.y > 0 && pos.y < window_states[window].window_height - 1) return pos;
	else return (struct point2d_int) { -1, -1 };
}

void window_set_mouse_cursor_position(uint32_t window, int32_t x, int32_t y) {
	POINT position;
	GetCursorPos(&position);
	RECT window_rect;
	GetWindowRect(window_states[window].hwnd, &window_rect);

	SetCursorPos(x + window_rect.left + 7, y + window_rect.top + 29);
}

VkResult create_vulkan_surface(VkInstance instance, uint32_t window, VkSurfaceKHR* surface) {
	VkWin32SurfaceCreateInfoKHR create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	create_info.hwnd = window_states[window].hwnd;
	create_info.hinstance = GetModuleHandleA(NULL);

	return vkCreateWin32SurfaceKHR(instance, &create_info, NULL, surface);
}

VkResult destroy_vulkan_surface(VkInstance instance, VkSurfaceKHR surface) {
	vkDestroySurfaceKHR(instance, surface, 0);

	return VK_SUCCESS;
}

int32_t _map_key(int32_t key) {

	switch (key) {

	case '1': return KEY_1;
	case '2': return KEY_2;
	case '3': return KEY_3;
	case '4': return KEY_4;
	case '5': return KEY_5;
	case '6': return KEY_6;
	case '7': return KEY_7;
	case '8': return KEY_8;
	case '9': return KEY_9;
	case '0': return KEY_0;

	case 'A': return KEY_A;
	case 'B': return KEY_B;
	case 'C': return KEY_C;
	case 'D': return KEY_D;
	case 'E': return KEY_E;
	case 'F': return KEY_F;
	case 'G': return KEY_G;
	case 'H': return KEY_H;
	case 'I': return KEY_I;
	case 'J': return KEY_J;
	case 'K': return KEY_K;
	case 'L': return KEY_L;
	case 'M': return KEY_M;
	case 'N': return KEY_N;
	case 'O': return KEY_O;
	case 'P': return KEY_P;
	case 'Q': return KEY_Q;
	case 'R': return KEY_R;
	case 'S': return KEY_S;
	case 'T': return KEY_T;
	case 'U': return KEY_U;
	case 'V': return KEY_V;
	case 'W': return KEY_W;
	case 'X': return KEY_X;
	case 'Y': return KEY_Y;
	case 'Z': return KEY_Z;

	case VK_ADD: return KEY_PLUS;
	case VK_SUBTRACT: return KEY_MINUS;

	case VK_F1: return KEY_F1;
	case VK_F2: return KEY_F2;
	case VK_F3: return KEY_F3;
	case VK_F4: return KEY_F4;
	case VK_F5: return KEY_F5;
	case VK_F6: return KEY_F6;
	case VK_F7: return KEY_F7;
	case VK_F8: return KEY_F8;
	case VK_F9: return KEY_F9;
	case VK_F10: return KEY_F10;
	case VK_F11: return KEY_F11;
	case VK_F12: return KEY_F12;
	
	case VK_LSHIFT: return KEY_SHIFT_L;
	case VK_RSHIFT: return KEY_SHIFT_R;
	case VK_LCONTROL: return KEY_CTRL_L;
	case VK_RCONTROL: return KEY_CTRL_R;
	case VK_LMENU: return KEY_ALT_L;
	case VK_RMENU: return KEY_ALT_R;
	
	case VK_SPACE: return KEY_SPACE;
	case VK_BACK: return KEY_BACKSPACE;
	case VK_TAB: return KEY_TAB;
	case VK_RETURN: return KEY_ENTER;
	case VK_ESCAPE: return KEY_ESCAPE;

	case VK_UP: return KEY_ARROW_UP;
	case VK_DOWN: return KEY_ARROW_DOWN;
	case VK_LEFT: return KEY_ARROW_LEFT;
	case VK_RIGHT: return KEY_ARROW_RIGHT;

	case VK_LBUTTON: return KEY_MOUSE_LEFT;
	case VK_MBUTTON: return KEY_MOUSE_MIDDLE;
	case VK_RBUTTON: return KEY_MOUSE_RIGHT;

	default: return -1;
	}

}

LRESULT CALLBACK WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	for (uint32_t i = 0; i < MAX_WINDOW_COUNT; i++) if (window_states[i].hwnd == hwnd) {

		if (uMsg == WM_CLOSE) {
			window_event_queues[i].event_destroy = 1;
		}

		else if (uMsg == WM_SIZE || uMsg == WM_MOVE) {
			window_event_queues[i].event_move_size = 1;

			RECT rect;
			GetClientRect(hwnd, &rect);

			window_states[i].window_width = rect.right - rect.left;
			window_states[i].window_height = rect.bottom - rect.top;
			window_states[i].position_x = rect.left;
			window_states[i].position_y = rect.top;
		}

		else if (uMsg == WM_SETFOCUS) window_event_queues[i].event_focus = 1;

		else if (uMsg == WM_KILLFOCUS) window_event_queues[i].event_unfocus = 1;

		else if (uMsg == WM_MOUSEWHEEL) {
			window_event_queues[i].event_mouse_scroll = 1;
			window_event_queues[i].scroll_steps = GET_WHEEL_DELTA_WPARAM(wParam) / 120;
		}

		else if (uMsg == WM_KEYDOWN || uMsg == WM_LBUTTONDOWN || uMsg == WM_MBUTTONDOWN || uMsg == WM_RBUTTONDOWN) {

			uint32_t key_code;

			if (uMsg == WM_KEYDOWN) key_code = wParam;
			else if (uMsg == WM_LBUTTONDOWN) key_code = VK_LBUTTON;
			else if (uMsg == WM_MBUTTONDOWN) key_code = VK_MBUTTON;
			else if (uMsg == WM_RBUTTONDOWN) key_code = VK_RBUTTON;

			uint32_t key = _map_key(key_code);
			if (key == -1) break;

			window_event_queues[i].event_key_down = 1;
			window_event_queues[i].key = key;
		}

		else if (uMsg == WM_KEYUP || uMsg == WM_LBUTTONUP || uMsg == WM_MBUTTONUP || uMsg == WM_RBUTTONUP) {

			uint32_t key_code;

			if (uMsg == WM_KEYUP) key_code = wParam;
			else if (uMsg == WM_LBUTTONUP) key_code = VK_LBUTTON;
			else if (uMsg == WM_MBUTTONUP) key_code = VK_MBUTTON;
			else if (uMsg == WM_RBUTTONUP) key_code = VK_RBUTTON;

			uint32_t key = _map_key(key_code);
			if (key == -1) break;

			window_event_queues[i].event_key_up = 1;
			window_event_queues[i].key = key;
		}

		else if (uMsg == WM_CHAR) {
			window_event_queues[i].event_char = 1;
			window_event_queues[i].unicode = wParam;
		}

		else return DefWindowProcW(hwnd, uMsg, wParam, lParam);

		return 0;
	}

	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

uint32_t _get_event(uint32_t window, struct window_event* event) {
	
	if (window_event_queues[window].event_destroy) {
		window_event_queues[window].event_destroy = 0;
		event->type = WINDOW_EVENT_DESTROY;
	}
	else if (window_event_queues[window].event_unfocus) {
		window_event_queues[window].event_unfocus = 0;
		event->type = WINDOW_EVENT_UNFOCUS;
	}
	else if (window_event_queues[window].event_move_size) {
		window_event_queues[window].event_move_size = 0;
		event->type = WINDOW_EVENT_MOVE_SIZE;
		event->info.event_move_size.width = window_states[window].window_width;
		event->info.event_move_size.height = window_states[window].window_height;
		event->info.event_move_size.position_x = window_states[window].position_x;
		event->info.event_move_size.position_y = window_states[window].position_y;
	}
	else if (window_event_queues[window].event_focus) {
		window_event_queues[window].event_focus = 0;
		event->type = WINDOW_EVENT_FOCUS;
	}
	else if (window_event_queues[window].event_mouse_scroll) {
		window_event_queues[window].event_mouse_scroll = 0;
		event->type = WINDOW_EVENT_MOUSE_SCROLL;
		event->info.event_mouse_scroll.scroll_steps = window_event_queues[window].scroll_steps;
	}
	else if (window_event_queues[window].event_char) {
		window_event_queues[window].event_char = 0;
		event->type = WINDOW_EVENT_CHAR;
		event->info.event_char.unicode = window_event_queues[window].unicode;
	}
	else if (window_event_queues[window].event_key_down) {
		window_event_queues[window].event_key_down = 0;
		event->type = WINDOW_EVENT_KEY_DOWN;
		event->info.event_key_down.key = window_event_queues[window].key;
	}
	else if (window_event_queues[window].event_key_up) {
		window_event_queues[window].event_key_up = 0;
		event->type = WINDOW_EVENT_KEY_UP;
		event->info.event_key_up.key = window_event_queues[window].key;
	}
	else return 0;

	return 1;
}

uint32_t window_process_next_event(uint32_t window, struct window_event* event) {
	
	struct window_event e;
	MSG message;
	uint32_t _get_event_return_status = 0;

	while ((_get_event_return_status = _get_event(window, &e)) == 0) {

		if (PeekMessageW(&message, window_states[window].hwnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessageW(&message);
		}
		else break;
	}

	if (_get_event_return_status) {
		if (event) *event = e;
		return 1;
	}
	else return 0;
}

void platform_init() {

	AllocConsole();
	console = GetConsoleWindow();
	ShowWindow(console, SW_HIDE);

	FILE* fstdout;
	freopen_s(&fstdout, "CONOUT$", "w", stdout);

	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);

	setvbuf(stdout, NULL, _IONBF, 0);

	DWORD dwMode = 0;
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	if (GetConsoleMode(hStdOut, &dwMode)) {
		dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(hStdOut, dwMode);
	}

	wc = (WNDCLASSW){
		CS_HREDRAW | CS_VREDRAW | CS_CLASSDC,
		WinProc,
		0,
		0,
		GetModuleHandleW(NULL),
		NULL,
		LoadCursorW(NULL, IDC_ARROW),
		NULL,
		NULL,
		L"BasicWindowClass"
	};

	RegisterClassW(&wc);

	return;
}

void platform_exit() {
	for (int32_t i = 0; i < MAX_WINDOW_COUNT; i++) window_destroy(i);

	UnregisterClassW(L"BasicWindowClass", GetModuleHandleA(NULL));
	FreeConsole();
}