#include <utils.h>

#include <stdio.h>

#ifdef WIN32

#include <windows.h>
#include <direct.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

void* dynamic_library_load(uint8_t* library_name, bool name_explicit)
{
	uint8_t* load_library_name = library_name;

	if (name_explicit == false)
	{
		size_t library_name_length = strlen(library_name);
		load_library_name = malloc(library_name_length + sizeof(".dll"));

		memcpy(load_library_name, library_name, library_name_length);
		memcpy(&load_library_name[library_name_length], ".dll", sizeof(".dll"));
	}

	void* handle = LoadLibraryA(load_library_name);

	if (name_explicit == false) free(load_library_name);

	return handle;
}

void (*dynamic_library_get_function(void* library_handle, uint8_t* function_name)) (void)
{
	return (void (*)(void)) GetProcAddress(library_handle, function_name);
}

void dynamic_library_unload(void* library_handle)
{
	FreeLibrary(library_handle);
}

#else

#include <malloc.h>
#include <dlfcn.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

void* dynamic_library_load(uint8_t* src)
{

	int32_t src_length = 0;
	for (; src[src_length] != 0; src_length++);

	char* src_platform = alloca(sizeof("lib") - 1 + src_length + sizeof(".so"));

	src_platform[0] = 'l';
	src_platform[1] = 'i';
	src_platform[2] = 'b';
	for (int32_t i = 3; i < src_length + 3; i++) src_platform[i] = src[i - 3];
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

void (*dynamic_library_get_function(void* library_handle, uint8_t* function_name)) (void)
{
	return dlsym(library_handle, function_name);
}

void dynamic_library_unload(void* library_handle)
{
	dlclose(library_handle);
}


#endif

#ifdef WIN32

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
#else

uint32_t directory_exists(uint8_t* path)
{

}

uint32_t create_directory(uint8_t* path)
{

}

uint32_t get_cwd(uint8_t* buffer, size_t buffer_size)
{

}

void set_console_cursor_position(int32_t x, int32_t y)
{
	printf("\033[%d;%dH", y, x);
}

void sleep_for_ms(uint32_t time_in_milliseconds)
{
	usleep(time_in_milliseconds * 1000);
}

double get_time()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double)tv.tv_sec * 1000. + (double)tv.tv_usec / 1000.;
}

void* create_thread(void (address)(void*), void* args)
{
	pthread_t* thread = malloc(sizeof(pthread_t));
	pthread_create(thread, NULL, (void* (*)(void*))address, args);
	return thread;
}

void join_thread(void* thread_handle)
{
	pthread_join(*(pthread_t*)thread_handle, NULL);
	free(thread_handle);
}

#endif


uint32_t file_copy(uint8_t* source_path, uint8_t* dest_path)
{
	FILE* source = fopen(source_path, "rb");
	if (source == NULL) {
		return 1;
	}

	FILE* dest = fopen(dest_path, "wb");
	if (dest == NULL) {
		fclose(source);
		return 1;
	}

	char buffer[4096];
	size_t bytes;

	while ((bytes = fread(buffer, 1, sizeof(buffer), source)) > 0) {
		if (fwrite(buffer, 1, bytes, dest) != bytes) {
			fclose(source);
			fclose(dest);
			return 1;
		}
	}

	fclose(source);
	fclose(dest);
	return 0;
}

void* file_load(uint8_t* path, size_t* size)
{

	FILE* file = fopen(path, "rb");

	if (file == NULL) return NULL;

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	rewind(file);

	void* buffer = malloc(fileSize);
	if (buffer == NULL) {
		fclose(file);
		return NULL;
	}

	size_t read = fread(buffer, 1, fileSize, file);

	fclose(file);

	if (read != fileSize)
	{
		free(buffer);
		return NULL;
	}

	*size = read;

	return buffer;
}