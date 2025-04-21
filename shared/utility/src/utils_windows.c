#include "utils.h"

#include <windows.h>
#include <direct.h>
#include <limits.h>
#include <malloc.h>

void* dynamic_library_load(uint8_t* src)
{

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

void (*dynamic_library_get_function(void* library_handle, uint8_t* function_name)) (void) 
{
	return (void (*)(void)) GetProcAddress(library_handle, function_name);
}

void dynamic_library_unload(void* library_handle) 
{
	FreeLibrary(library_handle);
}

uint32_t directory_exists(uint8_t* path)
{
	DWORD attr = GetFileAttributes(path);
	return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY));
}

uint32_t create_directory(uint8_t* path)
{
	if (_mkdir(path) == 0) return 0;
	else return 1;	
}

uint32_t get_cwd(uint8_t* buffer, size_t buffer_size)
{
	if (_getcwd(buffer, buffer_size) != NULL)
	{
		for (uint32_t i = 0; buffer[i] != '\0'; i++)
			if (buffer[i] == '\\') buffer[i] = '/';

		return 0;
	}
	else return 1;
}

void set_console_cursor_position(int32_t x, int32_t y) 
{
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD) { (SHORT)x, (SHORT)y });
}

void sleep_for_ms(uint32_t time_in_milliseconds) 
{
	Sleep(time_in_milliseconds);
}

double get_time() 
{
	LARGE_INTEGER frequency, start;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&start);

	return (double)1000 * ((double)start.QuadPart / (double)frequency.QuadPart);
}

void* create_thread(void (address)(void*), void* args) 
{
	return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)address, args, 0, NULL);
}

void join_thread(void* thread_handle) 
{
	WaitForSingleObject(thread_handle, INFINITE);
	CloseHandle(thread_handle);
}