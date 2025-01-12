#include "platform.h"

#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

struct window_state {
	HWND hwnd;
	uint8_t active;
	uint8_t registered_active;
	int32_t window_width;
	int32_t window_height;
	int32_t position_x;
	int32_t position_y;
};

uint32_t initialized = 0;

HWND console;

struct window_state window_states[MAX_WINDOW_COUNT];
WNDCLASSW wc;

struct window_event* we = NULL;
uint32_t is_trackable_window_event = 0;
uint32_t window_check = 0;

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
	if (initialized) ShowWindow(console, SW_SHOW);;
}

void hide_console_window() {
	if (initialized) ShowWindow(console, SW_HIDE);
}

uint32_t window_create(uint32_t posx, uint32_t posy, uint32_t width, uint32_t height, uint8_t* name, uint32_t visible) {

	uint32_t next_free_window_index = 0;
	for (; next_free_window_index < MAX_WINDOW_COUNT; next_free_window_index++) if (window_states[next_free_window_index].hwnd == NULL) break;
	if (next_free_window_index == MAX_WINDOW_COUNT) return WINDOW_CREATION_FAILED;

	int32_t name_length = 1;
	for (; name[name_length - 1] != '\0'; name_length++);

	USHORT* wide_name = alloca(name_length * sizeof(USHORT));

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

	SendMessageW(window_states[next_free_window_index].hwnd, WM_SIZE, 0, 0);

	window_states[next_free_window_index].window_width = width;
	window_states[next_free_window_index].window_height = height;
	window_states[next_free_window_index].position_x = posx;
	window_states[next_free_window_index].position_y = posy;
	window_states[next_free_window_index].active = 1;
	window_states[next_free_window_index].registered_active = 1;

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

int32_t window_is_active(uint32_t window) {
	return window_states[window].active;
}

void window_destroy(uint32_t window) {
	if (window_states[window].active) SendMessage(window_states[window].hwnd, WM_CLOSE, 0, 0);
	window_states[window].active = 0;
	window_states[window].registered_active = 0;
	window_states[window].hwnd = NULL;
}

struct point2d_int window_get_mouse_cursor_position(uint32_t window) {
	if (!window_states[window].active) return (struct point2d_int) { -1, -1 };

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
	case VK_F12: return KEY_F12
		;
	
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

	default: return -1;
	}

}

LRESULT CALLBACK WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch (uMsg) {

	case WM_CLOSE: {
		DestroyWindow(hwnd);
	} break;

	case WM_DESTROY: {
		is_trackable_window_event = 1;

		PostQuitMessage(0);
		window_states[window_check].active = 0;

		if (we != NULL) we->type = WINDOW_EVENT_DESTROY;
	} break;

	case WM_SIZE: {

		is_trackable_window_event = 1;

		RECT rect;
		GetClientRect(hwnd, &rect);
		window_states[window_check].window_width = rect.right - rect.left;
		window_states[window_check].window_height = rect.bottom - rect.top;

		if (we != NULL) { 
			we->type = WINDOW_EVENT_MOVE_SIZE;

			we->info.event_move_size.position_x = window_states[window_check].position_x;
			we->info.event_move_size.position_y = window_states[window_check].position_y;
			we->info.event_move_size.width = window_states[window_check].window_width;
			we->info.event_move_size.height = window_states[window_check].window_height;
		}
	} break;

	case WM_MOVE: {
		is_trackable_window_event = 1;
		RECT rect;
		GetWindowRect(hwnd, &rect);

		window_states[window_check].position_x = rect.left;
		window_states[window_check].position_y = rect.top;

		if (we != NULL) {
			we->type = WINDOW_EVENT_MOVE_SIZE;

			we->info.event_move_size.position_x = window_states[window_check].position_x;
			we->info.event_move_size.position_y = window_states[window_check].position_y;
			we->info.event_move_size.width = window_states[window_check].window_width;
			we->info.event_move_size.height = window_states[window_check].window_height;
		}
	} break;

	case WM_SETFOCUS: {
		is_trackable_window_event = 1;
		if (we != NULL) we->type = WINDOW_EVENT_FOCUS;
	} break;

	case WM_KILLFOCUS: {
		is_trackable_window_event = 1;
		if (we != NULL) we->type = WINDOW_EVENT_UNFOCUS;
	} break;


	case WM_MOUSEWHEEL: {
		is_trackable_window_event = 1;

		if (we != NULL) {
			we->type = WINDOW_EVENT_MOUSE_SCROLL;
			we->info.event_mouse_scroll.scroll_steps = GET_WHEEL_DELTA_WPARAM(wParam) / 120;
		}
			
	} break;

	case WM_CHAR: {
		is_trackable_window_event = 1;

		WCHAR utf16_char = (WCHAR)wParam;
		uint32_t utf8_char = 0;
		uint32_t bytes_written = WideCharToMultiByte(CP_UTF8, 0, &utf16_char, 1, &utf8_char, sizeof(utf8_char), NULL, NULL);

		if (we != NULL) { 
			we->type = WINDOW_EVENT_CHAR;
			we->info.event_char.unicode = utf16_char;
		}
	} break;

	case WM_KEYDOWN: {
		int32_t key = _map_key(wParam);
		if (key == -1) break;

		is_trackable_window_event = 1;

		if (we != NULL) {
			we->type = WINDOW_EVENT_KEY_DOWN;
			we->info.event_key_down.key = key;
		}
	} break;

	case WM_KEYUP: {
		int32_t key = _map_key(wParam);
		if (key == -1) break;

		is_trackable_window_event = 1;

		if (we != NULL) {
			we->type = WINDOW_EVENT_KEY_UP;
			we->info.event_key_up.key = key;
		}
	} break;

	//mouse buttons
	case WM_LBUTTONDOWN: {
		is_trackable_window_event = 1;

		if (we != NULL) {
			we->type = WINDOW_EVENT_KEY_DOWN;
			we->info.event_key_down.key = KEY_MOUSE_LEFT;
		}
	} break;

	case WM_LBUTTONUP: {
		is_trackable_window_event = 1;

		if (we != NULL) {
			we->type = WINDOW_EVENT_KEY_UP;
			we->info.event_key_up.key = KEY_MOUSE_LEFT;
		}
	} break;

	case WM_MBUTTONDOWN: {
		is_trackable_window_event = 1;

		if (we != NULL) {
			we->type = WINDOW_EVENT_KEY_DOWN;
			we->info.event_key_down.key = KEY_MOUSE_MIDDLE;
		}
	} break;

	case WM_MBUTTONUP: {
		is_trackable_window_event = 1;

		if (we != NULL) {
			we->type = WINDOW_EVENT_KEY_UP;
			we->info.event_key_up.key = KEY_MOUSE_MIDDLE;
		}
	} break;

	case WM_RBUTTONDOWN: {
		is_trackable_window_event = 1;

		if (we != NULL) {
			we->type = WINDOW_EVENT_KEY_DOWN;
			we->info.event_key_down.key = KEY_MOUSE_RIGHT;
		}
	} break;

	case WM_RBUTTONUP: {
		is_trackable_window_event = 1;

		if (we != NULL) {
			we->type = WINDOW_EVENT_KEY_UP;
			we->info.event_key_up.key = KEY_MOUSE_RIGHT;
		}
	} break;

	}

	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

uint32_t window_process_next_event(uint32_t window, struct window_event* event) {
	
	MSG message;
	we = event;
	window_check = window;

	while (window_states[window].registered_active) {

		if (window_states[window].active == 0) {

			window_states[window].registered_active = 0;
			if (we) we->type = WINDOW_EVENT_DESTROY;

			we = NULL;
			return 1;
		}

		is_trackable_window_event = 0;

		if (PeekMessageW(&message, window_states[window].hwnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessageW(&message);

			if (is_trackable_window_event) {

				if (window_states[window].active == 0) window_states[window].registered_active = 0;

				we = NULL;
				return 1;
			}
		}

		else break;
	}
	
	we = NULL;
	return 0;
}

void platform_init() {
	initialized = 1;

	AllocConsole();
	console = GetConsoleWindow();
	ShowWindow(console, SW_HIDE);

	FILE* fstdout;
	freopen_s(&fstdout, "CONOUT$", "w", stdout);
	FILE* fstderr;
	freopen_s(&fstderr, "CONOUT$", "w", stderr);
	FILE* fstdin;
	freopen_s(&fstdin, "CONIN$", "r", stdin);

	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);

	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

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
	initialized = 0;

	for (int32_t i = 0; i < MAX_WINDOW_COUNT; i++) if (window_states[i].active) window_destroy(i);

	UnregisterClassA(L"BasicWindowClass", GetModuleHandleA(NULL));
	FreeConsole();
}