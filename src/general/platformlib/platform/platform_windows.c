#include "platform.h"

#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

struct window_state {
	HWND hwnd;
	uint16_t active;
	int32_t window_width;
	int32_t window_height;
	int32_t x_position;
	int32_t y_position;
};

uint32_t screen_width;
uint32_t screen_height;

struct window_state window_states[MAX_WINDOW_COUNT];
WNDCLASSW wc;
uint8_t keyStates[256] = { 0 };
static uint8_t running = 1;

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

void show_console_window() {
	HWND hwndConsole = GetConsoleWindow();
	if (hwndConsole != NULL) {
		ShowWindow(hwndConsole, SW_SHOW);
	}
}

void hide_console_window() {
	HWND hwndConsole = GetConsoleWindow();
	if (hwndConsole != NULL) {
		ShowWindow(hwndConsole, SW_HIDE);
	}
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

char get_key_state(int32_t key) {

	int8_t keyState = 0;

	SHORT currentKeyState = GetKeyState(key);

	if (currentKeyState & 0x8000) keyState |= 0b0001;
	if ((currentKeyState & 0x8000 ? 0x1 : 0x0) != keyStates[key]) keyState |= 0b0010;

	keyStates[key] = (currentKeyState & 0x8000 ? 0x1 : 0x0);

	return keyState;
}

uint32_t get_screen_width() {
	return screen_width;
}

uint32_t get_screen_height() {
	return screen_width;
}

//window functions

uint32_t window_create(uint32_t posx, uint32_t posy, uint32_t width, uint32_t height, uint8_t* name, uint32_t visible) {

	uint32_t next_free_window_index = 0;
	for (; next_free_window_index < MAX_WINDOW_COUNT; next_free_window_index++) if (window_states[next_free_window_index].hwnd == NULL) break;
	if (next_free_window_index == MAX_WINDOW_COUNT) return WINDOW_CREATION_FAILED;

	window_check = next_free_window_index;

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

	SendMessageA(window_states[next_free_window_index].hwnd, WM_SIZE, 0, 0);

	window_states[next_free_window_index].window_width = width;
	window_states[next_free_window_index].window_height = height;
	window_states[next_free_window_index].active = 1;

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
	window_states[window].hwnd = NULL;
}

void window_draw(uint32_t window, uint8_t* buffer, int32_t width, int32_t height, int32_t scalar) {

	if (!window_states[window].active) return;

	HDC hdc = GetDC(window_states[window].hwnd);

	BITMAPINFO bitmapInfo = { 0 };
	bitmapInfo.bmiHeader.biWidth = width;
	bitmapInfo.bmiHeader.biHeight = -height;
	bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFO);
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;

	SetStretchBltMode(hdc, COLORONCOLOR);
	StretchDIBits(hdc, 0, 0, width * scalar, height * scalar, 0, 0, width, height, buffer, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);

	ReleaseDC(window_states[window].hwnd, hdc);
}

struct point2d_int window_get_mouse_cursor_position(uint32_t window) {
	if (!window_states[window].active) return (struct point2d_int) { -1, -1 };

	POINT position;
	GetCursorPos(&position);
	RECT window_rect;
	GetWindowRect(window_states[window].hwnd, &window_rect);

	struct point2d_int pos = { position.x - window_rect.left - 7, position.y - window_rect.top - 29 };
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

LRESULT CALLBACK WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (uMsg == WM_CLOSE) { 
		DestroyWindow(hwnd); 
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}

	switch (uMsg) {

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
			we->type = WINDOW_EVENT_SIZE; 
			we->info.window_event_size.width = rect.right - rect.left;
			we->info.window_event_size.height = rect.bottom - rect.top;
		}
	} break;

	case WM_MOVE: {
		is_trackable_window_event = 1;
		RECT rect;
		GetWindowRect(hwnd, &rect);

		window_states[window_check].x_position = rect.left;
		window_states[window_check].y_position = rect.top;

		if (we != NULL) {
			we->type = WINDOW_EVENT_MOVE;
			we->info.window_event_move.x_position = rect.left;
			we->info.window_event_move.y_position = rect.top;
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
			we->info.window_event_mouse_scroll.scroll_steps = GET_WHEEL_DELTA_WPARAM(wParam) / 120;
		}
			
	} break;

	case WM_CHAR: {
		is_trackable_window_event = 1;

		WCHAR utf16_char = (WCHAR)wParam;
		uint32_t utf8_char = 0;
		uint32_t bytes_written = WideCharToMultiByte(CP_UTF8, 0, &utf16_char, 1, &utf8_char, sizeof(utf8_char), NULL, NULL);

		if (we != NULL) { 
			we->type = WINDOW_EVENT_CHAR;
			we->info.window_event_char.unicode = utf16_char;
		}
	} break;

	}

	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

uint32_t window_process_next_event(struct window_event* event) {
	MSG message;
	we = event;

	uint32_t first_window_checked = window_check;

	for (; window_check != (first_window_checked - 1 + MAX_WINDOW_COUNT) % MAX_WINDOW_COUNT; window_check = (window_check + 1) % MAX_WINDOW_COUNT) {


		while (window_states[window_check].active) {

			is_trackable_window_event = 0;

			if (PeekMessageA(&message, window_states[window_check].hwnd, 0, 0, PM_REMOVE)) {
				TranslateMessage(&message);
				DispatchMessageW(&message);

				if (is_trackable_window_event) {
					if (we) we->window = window_check;
					we = NULL;
					return 1;
				}
			}

			else break;
		}
	}
	
	return 0;
}

void platform_init() {

	if (!AllocConsole()) {
		MessageBox(NULL, "Failed to allocate console.", "Error", MB_OK | MB_ICONERROR);
		return;
	}

	hide_console_window();

	screen_width = GetSystemMetrics(SM_CXSCREEN);
	screen_height = GetSystemMetrics(SM_CYSCREEN);

	FILE* fstdout;
	freopen_s(&fstdout, "CONOUT$", "w", stdout);
	FILE* fstderr;
	freopen_s(&fstderr, "CONOUT$", "w", stderr);
	FILE* fstdin;
	freopen_s(&fstdin, "CONIN$", "r", stdin);

	// Set console to UTF-8
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);

	// Optional: Set no buffering for immediate output
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	DWORD dwMode = 0;
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	// Get current console mode
	if (GetConsoleMode(hStdOut, &dwMode)) {
		// Enable virtual terminal processing
		dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(hStdOut, dwMode);
	}

	wc = (WNDCLASSW){
		CS_HREDRAW | CS_VREDRAW | CS_CLASSDC,
		WinProc,
		0,
		0,
		GetModuleHandleA(NULL),
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
	for (int32_t i = 0; i < MAX_WINDOW_COUNT; i++) if (window_states[i].active) window_destroy(i);

	UnregisterClassA(L"BasicWindowClass", GetModuleHandleA(NULL));
	FreeConsole();
}