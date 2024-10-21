#include "platform.h"

#include <malloc.h>

#if defined(_WIN32)

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <stdbool.h>


struct window_resource {
	HWND hwnd;
	bool active;
	int window_width;
	int window_height;
	int mouse_scrolls;
	void (*char_callbacks[MAX_CALLBACK_FUNCTIONS]) (int, int);
	void (*key_down_callbacks[MAX_CALLBACK_FUNCTIONS]) (int, int);
};

static struct window_resource* window_resources[MAX_WINDOW_COUNT];

static struct {
	int posx;
	int posy;
	int width;
	int height;
	int parse_buffer_size;
	unsigned char* name;
	bool done_flag;
	int window_resources_index;
} next_window;

static WNDCLASSW wc;

static bool keyStates[256] = { 0 };

static bool running = true;

static void* window_control_thread;

void* dynamic_library_load(char* src) {

	int src_length = 0;
	for (; src[src_length] != 0; src_length++);

	char* src_platform = alloca(src_length + sizeof(".dll"));

	for (int i = 0; i < src_length; i++) src_platform[i] = src[i];
	src_platform[src_length] = '.';
	src_platform[src_length + 1] = 'd';
	src_platform[src_length + 2] = 'l';
	src_platform[src_length + 3] = 'l';
	src_platform[src_length + 4] = '\0';

	return  LoadLibraryA(src_platform);

}

void (*dynamic_library_get_function(void* library_handle, char* function_name)) (void) {
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

void set_console_cursor_position(int x, int y) {
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD) { (SHORT)x, (SHORT)y });
}

void sleep_for_ms(unsigned int _time_in_milliseconds) {
	Sleep(_time_in_milliseconds);
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

char get_key_state(int key) {

	char keyState = 0;

	SHORT currentKeyState = GetKeyState(key);

	if (currentKeyState & 0x8000) keyState |= 0b0001;
	if ((currentKeyState & 0x8000 ? 0x1 : 0x0) != keyStates[key]) keyState |= 0b0010;

	keyStates[key] = (currentKeyState & 0x8000 ? 0x1 : 0x0);

	return keyState;
}

//window functions

int window_create(int posx, int posy, int width, int height, unsigned char* name) {

	int next_free_window_index = 0;
	for (; next_free_window_index < MAX_WINDOW_COUNT; next_free_window_index++) if (window_resources[next_free_window_index] == NULL) break;
	if (next_free_window_index == MAX_WINDOW_COUNT) return WINDOW_CREATION_FAILED;


	next_window.posx = posx;
	next_window.posy = posy;
	next_window.width = width;
	next_window.height = height;
	next_window.name = name;
	next_window.done_flag = false;
	next_window.window_resources_index = next_free_window_index;

	while (next_window.done_flag == false) Sleep(1);

	return next_free_window_index;
}

int window_get_width(int window) {
	return window_resources[window]->window_width; 
}

int window_get_height(int window) { 
	return window_resources[window]->window_height; 
}

int window_is_selected(int window) { 
	return window_resources[window]->hwnd == GetForegroundWindow(); 
}

int window_is_active(int window) { 
	return window_resources[window]->active; 
}

void window_destroy(int window) {
	if (window_resources[window]->active) SendMessage(window_resources[window]->hwnd, WM_CLOSE, 0, 0);
	while (window_resources[window]->active) Sleep(1);
	free(window_resources[window]);
	window_resources[window] = NULL;
}

void window_draw(int window, unsigned int* buffer, int width, int height, int scalar) {

	if (window_resources[window]->active == false) return;

	HDC hdc = GetDC(window_resources[window]->hwnd);

	BITMAPINFO bitmapInfo;

	bitmapInfo.bmiHeader.biWidth = width;
	bitmapInfo.bmiHeader.biHeight = -height;
	bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFO);
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;

	SetStretchBltMode(hdc, COLORONCOLOR);
	StretchDIBits(hdc, 0, 0, width * scalar, height * scalar, 0, 0, width, height, buffer, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);

	ReleaseDC(window_resources[window]->hwnd, hdc);

}

struct point2d_int window_get_mouse_cursor_position(int window) {
	if (window_resources[window]->hwnd != GetForegroundWindow()) return (struct point2d_int) {-1, -1};

	POINT position;
	GetCursorPos(&position);
	RECT window_rect;
	GetWindowRect(window_resources[window]->hwnd, &window_rect);

	struct point2d_int pos = { position.x - window_rect.left - 7, position.y - window_rect.top - 29 };
	if(pos.x > 0 && pos.x < window_resources[window]->window_width - 1 && pos.y > 0 && pos.y < window_resources[window]->window_height - 1) return pos;
	else return (struct point2d_int) { -1, -1 };
}

void window_set_mouse_cursor_position(int window, int x, int y) {
	POINT position;
	GetCursorPos(&position);
	RECT window_rect;
	GetWindowRect(window_resources[window]->hwnd, &window_rect);

	SetCursorPos(x + window_rect.left + 7, y + window_rect.top + 29);
}

int window_get_last_mouse_scrolls(int window) {
	int temp = window_resources[window]->mouse_scrolls;
	window_resources[window]->mouse_scrolls = 0;
	return temp;
}

void window_clear_mouse_scrolls(int window) {
	window_resources[window]->mouse_scrolls = 0;
}

int window_add_char_callback(int window, void (*callback) (int, int)) {
	int next_free_callback_index = 0;
	for (; next_free_callback_index < MAX_WINDOW_COUNT; next_free_callback_index++) if (window_resources[window]->char_callbacks[next_free_callback_index] == NULL) break;
	if (next_free_callback_index == MAX_WINDOW_COUNT) return WINDOW_CREATION_FAILED;

	window_resources[window]->char_callbacks[next_free_callback_index] = callback;

	return next_free_callback_index;
}

void window_remove_char_callback(int window, int char_callback_id) {
	window_resources[window]->char_callbacks[char_callback_id] = NULL;
}

VkResult create_vulkan_surface(VkInstance instance, int window, VkSurfaceKHR* surface) {
	VkWin32SurfaceCreateInfoKHR create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	create_info.hwnd = window_resources[window]->hwnd;
	create_info.hinstance = GetModuleHandleA(NULL);

	return vkCreateWin32SurfaceKHR(instance, &create_info, NULL, surface);
}

void WindowControl() {
	while (running) {

		for (int i = 0; i < MAX_WINDOW_COUNT; i++) {

			MSG message;

			while (window_resources[i] != NULL) {
				if (PeekMessageW(&message, window_resources[i]->hwnd, 0, 0, PM_REMOVE)) {
					TranslateMessage(&message);
					DispatchMessageW(&message);
				}
				else break;
			}
		}

		//creating window

		if (next_window.done_flag == false) {
			int name_length = 1;

			for (; next_window.name[name_length - 1] != '\0'; name_length++);

			struct window_resource* window_resource = malloc(sizeof(struct window_resource) + name_length * sizeof(unsigned short));

			unsigned short* name_short = (long long)window_resource + sizeof(struct window_resource);

			for (int i = 0; i < name_length; i++) name_short[i] = (*(unsigned short*)&next_window.name[i] & 255);

			HWND window = CreateWindowExW(
				0,
				wc.lpszClassName,
				name_short,
				WS_OVERLAPPEDWINDOW | WS_VISIBLE,
				next_window.posx,
				next_window.posy,
				next_window.width,
				next_window.height,
				NULL,
				NULL,
				GetModuleHandleA(NULL),
				NULL
			);

			
			window_resource->hwnd = window;
			window_resource->active = true;

			for (int i = 0; i < MAX_CALLBACK_FUNCTIONS; i++) {
				window_resource->char_callbacks[i] = NULL;
				window_resource->key_down_callbacks[i] = NULL;
			}

			window_resources[next_window.window_resources_index] = window_resource;

			SendMessage(window, WM_SIZE, 0, 0);
			next_window.done_flag = true;
		}

		Sleep(10);
	}

	return;
}

LRESULT CALLBACK WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	for (int i = 0; i < MAX_WINDOW_COUNT; i++) if(window_resources[i] != NULL) {

		if (window_resources[i]->hwnd == hwnd && window_resources[i]->hwnd != NULL) {

			switch (uMsg) {

			case WM_CLOSE: {
				DestroyWindow(hwnd);
			} break;


			case WM_DESTROY: {
				PostQuitMessage(0);
				window_resources[i]->active = false;
			} break;

			case WM_SIZE: {
				RECT rect;
				GetClientRect(hwnd, &rect);
				window_resources[i]->window_width = rect.right - rect.left;
				window_resources[i]->window_height = rect.bottom - rect.top;
			} break;

			case WM_MOUSEWHEEL: {
				int wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);

				if (wheelDelta > 0) window_resources[i]->mouse_scrolls++;
				else if (wheelDelta < 0) window_resources[i]->mouse_scrolls--;
			} break;

			case WM_CHAR: {

				WCHAR utf16_char = (WCHAR)wParam; 
				int utf8_char = 0; 
				int bytes_written = WideCharToMultiByte(CP_UTF8, 0, &utf16_char, 1, &utf8_char, sizeof(utf8_char), NULL, NULL);

				for (int j = 0; j < MAX_CALLBACK_FUNCTIONS; j++) if (window_resources[i]->char_callbacks[j] != NULL) window_resources[i]->char_callbacks[j](i, utf8_char);

			} break;

			}

		}
	}

	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

void platform_init() {

	AllocConsole();
	hide_console_window();

	FILE* fstdout;
	freopen_s(&fstdout, "CONOUT$", "w", stdout);
	FILE* fstderr;
	freopen_s(&fstderr, "CONOUT$", "w", stderr);
	FILE* fstdin;
	freopen_s(&fstdin, "CONIN$", "r", stdin);

	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);

	fflush(stdout);
	fflush(stderr);
	fflush(stdin);

	wc = (WNDCLASSW) {
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

	next_window.done_flag = true;

	window_control_thread = create_thread(WindowControl, NULL);

	return;
}

void platform_exit() {
	for (int i = 0; i < MAX_WINDOW_COUNT; i++) if (window_resources[i] != NULL) window_destroy(i);
	running = false;
	join_thread(window_control_thread);
}

#elif defined(__linux__)

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/Xlocale.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

struct window_resource {
	Window window;
	XImage* image;
	XIC ic;
	unsigned int* pixels;
	int active;
	int drawing;
	int window_width;
	int window_height;
	int mouse_scrolls;
	void (*char_callbacks[MAX_CALLBACK_FUNCTIONS]) (int, int);
	void (*key_down_callbacks[MAX_CALLBACK_FUNCTIONS]) (int, int);
};

static struct window_resource* window_resources[MAX_WINDOW_COUNT];

static char window_resources_active[MAX_WINDOW_COUNT] = { 0 };

static int display_width;
static int display_height;

static Display* display;
static int screen;
static Atom wm_delete_window;
static XIM xim;

static int running = 1;

static char keyStates[256 * 256] = { 0 };
static int last_mouse_scroll = 0;

static char mouseButtons[3] = { 0, 0, 0 };

static void* window_control_thread;

void* dynamic_library_load(char* src) {

	int src_length = 0;
	for (; src[src_length] != 0; src_length++);

	char* src_platform = alloca(sizeof("lib") - 1 +src_length + sizeof(".so"));

	src_platform[0] = 'l';
	src_platform[1] = 'i';
	src_platform[2] = 'b';
	for (int i = 3; i < src_length + 3; i++) src_platform[i] = src[i - 3];
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

void (*dynamic_library_get_function(void* library_handle, char* function_name)) (void) {
	return dlsym(library_handle, function_name);
}

void dynamic_library_unload(void* library_handle) {
	dlclose(library_handle);
}

void show_console_window() { return; }

void hide_console_window() { return; }

void set_console_cursor_position(int x, int y) {
	printf("\033[%d;%dH", y, x);
}

void sleep_for_ms(unsigned int _time_in_milliseconds) {
	usleep(_time_in_milliseconds * 1000);
}

double get_time() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double)tv.tv_sec * 1000 + (double)tv.tv_usec / 1000;
}

void* create_thread(void (address)(void*), void* args) {
	pthread_t* thread = malloc(sizeof(pthread_t));
	pthread_create(thread, NULL, (void* (*)(void*))address, args);
	return thread;
}

void join_thread(void* thread_handle) {
	pthread_join(*(pthread_t*)thread_handle, NULL);
	free(thread_handle);
}

char get_key_state(int key) {

	char key_state = 0;

	if (key == KEY_MOUSE_LEFT || key == KEY_MOUSE_MIDDLE || key == KEY_MOUSE_RIGHT) {

		Window root = DefaultRootWindow(display);
		Window root_return, child_return;
		int root_x, root_y, win_x, win_y;
		unsigned int mask_return;
		XQueryPointer(display, root, &root_return, &child_return, &root_x, &root_y, &win_x, &win_y, &mask_return);

		if (mask_return & (key == KEY_MOUSE_LEFT ? Button1Mask : (key == KEY_MOUSE_MIDDLE ? Button2Mask : Button3Mask))) key_state = 0b1;

		if (key_state != mouseButtons[(key == KEY_MOUSE_LEFT ? 0 : (key == KEY_MOUSE_MIDDLE ? 1 : 2))]) key_state |= 0b10;

		mouseButtons[(key == KEY_MOUSE_LEFT ? 0 : (key == KEY_MOUSE_MIDDLE ? 1 : 2))] = key_state & 0b1;

		return key_state;
	}

	char keys[32];
	XQueryKeymap(display, keys);

	KeySym keysym = (KeySym)key;
	KeyCode keycode = XKeysymToKeycode(display, keysym);

	int byteIndex = keycode / 8;
	int bitIndex = keycode % 8;

	if (keys[byteIndex] & (1 << bitIndex)) key_state = 0b1;
	if (key_state != keyStates[key]) key_state |= 0b10;
	keyStates[key] = key_state & 0b1;

	return key_state;
}

int get_last_mouse_scroll() {
	int temp = last_mouse_scroll;
	last_mouse_scroll = 0;
	return temp;
}

void clear_mouse_scroll() { last_mouse_scroll = 0; }

int window_create(int posx, int posy, int width, int height, unsigned char* name) {

	int next_free_window_index = 0;
	for (; next_free_window_index < MAX_WINDOW_COUNT; next_free_window_index++) if (window_resources[next_free_window_index] == NULL) break;
	if (next_free_window_index == MAX_WINDOW_COUNT) return WINDOW_CREATION_FAILED;

	window_resources_active[next_free_window_index] = 1;

	Window window = XCreateSimpleWindow(display, RootWindow(display, screen), posx, posy, width, height, 1, BlackPixel(display, screen), WhitePixel(display, screen));

	XIC ic = XCreateIC(xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, window, NULL);

	unsigned int* pixels = malloc(display_width * display_height * sizeof(unsigned int));

	XImage* image = XCreateImage(display, DefaultVisual(display, screen), DefaultDepth(display, screen), ZPixmap, 0, (char*)pixels, display_width, display_height, 32, 0);

	struct window_resource* window_resource = malloc(sizeof(struct window_resource));


	window_resource->window = window;
	window_resource->ic = ic;
	window_resource->image = image;
	window_resource->pixels = pixels;
	window_resource->active = 1;
	window_resource->drawing = 0;
	window_resource->window_width = width;
	window_resource->window_height = height;

	XSelectInput(display, window, ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask | ButtonPressMask);
	XStoreName(display, window, name);
	XSetWMProtocols(display, window, &wm_delete_window, 1);
	XMapWindow(display, window);

	window_resources[next_free_window_index] = window_resource;

	return next_free_window_index;
}

int window_get_width(int window) {
	return window_resources[window]->window_width;
}

int window_get_height(int window) {
	return window_resources[window]->window_height;
}

int window_is_selected(int window) {
	Window focused_window;
	int revert_to;

	XGetInputFocus(display, &focused_window, &revert_to);

	return focused_window == window_resources[window]->window;
}

int window_is_active(int window) {
	return window_resources[window]->active;
}

void window_destroy(int window) {

	XDestroyIC(window_resources[window]->ic);
	if (window_resources[window]->active) XDestroyWindow(display, window_resources[window]->window);
	window_resources_active[window] = 0;

	XDestroyImage(window_resources[window]->image);
	free(window_resources[window]);
	window_resources[window] = NULL;
}

void window_draw(int window, unsigned int* buffer, int width, int height, int scalar) {

	window_resources[window]->drawing = 1;

	if (!window_resources[window]->active) return;
	for (int i = 0; i < width * scalar && i < display_width; i++) {
		for (int j = 0; j < height * scalar && j < display_height; j++) {
			window_resources[window]->pixels[i + display_width * j] = buffer[i / scalar + width * (j / scalar)];
		}
	}


	XPutImage(display, window_resources[window]->window, DefaultGC(display, screen), window_resources[window]->image, 0, 0, 0, 0, width * scalar, height * scalar);

	window_resources[window]->drawing = 0;

}

struct point2d_int window_get_mouse_cursor_position(int window) {
	if (!window_resources[window]->active) return (struct point2d_int) { -1, -1 };
	Window root, child;
	int root_x, root_y;
	int win_x, win_y;
	unsigned int mask;

	XQueryPointer(display, window_resources[window]->window, &root, &child, &root_x, &root_y, &win_x, &win_y, &mask);

	struct point2d_int pos = { win_x + 1, win_y + 1 };

	return pos;
}

void window_set_mouse_cursor_position(int window, int x, int y) {
	if (!window_resources[window]->active) return;
	Window root, child;
	int root_x, root_y;
	int win_x, win_y;
	unsigned int mask;

	XQueryPointer(display, window_resources[window]->window, &root, &child, &root_x, &root_y, &win_x, &win_y, &mask);
	XWarpPointer(display, None, DefaultRootWindow(display), 0, 0, 0, 0, root_x - win_x + x + 2, root_y - win_y + window_resources[window]->window_height - y + 1);
	XFlush(display);
	return;
}

int window_add_char_callback(int window, void (*callback) (int, int)) {
	int next_free_callback_index = 0;
	for (; next_free_callback_index < MAX_WINDOW_COUNT; next_free_callback_index++) if (window_resources[window]->char_callbacks[next_free_callback_index] == NULL) break;
	if (next_free_callback_index == MAX_WINDOW_COUNT) return WINDOW_CREATION_FAILED;

	window_resources[window]->char_callbacks[next_free_callback_index] = callback;

	return next_free_callback_index;
}

void window_remove_char_callback(int window, int char_callback_id) {
	window_resources[window]->char_callbacks[char_callback_id] = NULL;
}

VkResult create_vulkan_surface(VkInstance instance, int window, VkSurfaceKHR* surface) {
	VkXlibSurfaceCreateInfoKHR create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	create_info.window = window_resources[window]->hwnd;
	create_info.dpy = display;

	return vkCreateXlibSurfaceKHR(instance, &create_info, NULL, surface);
}


void WindowControl() {
	XEvent event;
	while (running) {

		while (XPending(display) && running) {

			XNextEvent(display, &event);

			int window_index = 0;

			for (; window_index < MAX_WINDOW_COUNT; window_index++) {
				if (window_resources[window_index]) {
					if (window_resources[window_index]->window == event.xany.window) break;
				}
				
			}

			if (window_index == MAX_WINDOW_COUNT) continue;

			if (window_resources[window_index]->active) {

				switch (event.type) {

				case ConfigureNotify: {
					window_resources[window_index]->window_width = (event.xconfigure.width > display_width ? display_width : event.xconfigure.width);
					window_resources[window_index]->window_height = (event.xconfigure.height > display_height ? display_height : event.xconfigure.height);
				} break;

				case ClientMessage: {
					if ((Atom)event.xclient.data.l[0] == wm_delete_window) {

						window_resources[window_index]->active = 0;
						while (window_resources[window_index]->drawing) usleep(10);
						
						XDestroyWindow(display, window_resources[window_index]->window);
					}
				} break;

				case ButtonPress: {
					if (event.xbutton.button == Button4) last_mouse_scroll++;
					else if (event.xbutton.button == Button5) last_mouse_scroll--;
				} break;

				case KeyPress: {
					KeySym keysym;
					char buf[32];
					XComposeStatus compose_status;
					int len = XmbLookupString(window_resources[window_index]->ic, &event, buf, sizeof(buf) - 1, &keysym, &compose_status);

					if (len > 0) {

						for (int j = 0; j < MAX_CALLBACK_FUNCTIONS; j++) if (window_resources[window_index]->char_callbacks[j] != NULL) window_resources[window_index]->char_callbacks[j](window_index, *((int*)&buf));

						
					}

				} break;

				}

			}

		}

		sleep_for_ms(10);
	}

	return;
}

void platform_init() {
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

	window_control_thread = create_thread(WindowControl, NULL);

	return;
}

void platform_exit() {
	running = 0;
	join_thread(window_control_thread);
	XCloseIM(xim);
	XCloseDisplay(display);
}

#endif 