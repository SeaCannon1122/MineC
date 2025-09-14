#include <utils.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef WIN32

#include <windows.h>
#include <direct.h>
#include <limits.h>

void* dynamic_library_load(uint8_t* library_name, bool name_explicit)
{
	uint8_t* load_library_name = library_name;

	if (name_explicit == false)
	{
		size_t library_name_length = strlen(library_name);


		if ((load_library_name = malloc(library_name_length + sizeof(".dll"))) == NULL) return NULL;

		snprintf(load_library_name, library_name_length + sizeof(".dll"), "%s.´dll", library_name);
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

void time_sleep(uint32_t time_in_milliseconds)
{
	Sleep(time_in_milliseconds);
}

LARGE_INTEGER frequency = { .QuadPart = 0 };

double time_get()
{
	LARGE_INTEGER start;
	if (frequency.QuadPart == 0) QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&start);

	return 1000.0 * (double)start.QuadPart / (double)frequency.QuadPart;
}

void* thread_create(void (address)(void*), void* args)
{
	return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)address, args, 0, NULL);
}

void thread_join(void* thread_handle)
{
	WaitForSingleObject(thread_handle, INFINITE);
	CloseHandle(thread_handle);
}

#else

#include <dlfcn.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

void* dynamic_library_load(uint8_t* library_name, bool name_explicit)
{

	uint8_t* load_library_name = library_name;

	if (name_explicit == false)
	{
		size_t library_name_length = strlen(library_name);
		if ((load_library_name = malloc(sizeof("lib") - 1 + library_name_length + sizeof(".so"))) == NULL) return NULL;

		snprintf(load_library_name, sizeof("lib") - 1 + library_name_length + sizeof(".so"), "lib%s.so", library_name);
	}

	void* handle = dlopen(load_library_name, RTLD_NOW);

	if (name_explicit == false) free(load_library_name);

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

void time_sleep(uint32_t time_in_milliseconds)
{
	usleep(time_in_milliseconds * 1000);
}

double time_get()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double)tv.tv_sec * 1000. + (double)tv.tv_usec / 1000.;
}

void* thread_create(void (address)(void*), void* args)
{
	pthread_t* thread = malloc(sizeof(pthread_t));
	pthread_create(thread, NULL, (void* (*)(void*))address, args);
	return thread;
}

void thread_join(void* thread_handle)
{
	pthread_join(*(pthread_t*)thread_handle, NULL);
	free(thread_handle);
}

#endif

uint32_t file_copy(uint8_t* src_path, uint8_t* dst_path)
{
	FILE* source = fopen(src_path, "rb");
	if (source == NULL) {
		return 1;
	}

	FILE* dest = fopen(dst_path, "wb");
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

	if (fileSize == 0)
	{
		fclose(file);
		return NULL;
	}

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

bool file_save(uint8_t* path, void* data, size_t size)
{
	if (path == NULL || data == NULL || size == 0) return false;

	FILE* file = fopen((const char*)path, "wb");
	if (file == NULL) return false;

	size_t written = fwrite(data, 1, size, file);
	fclose(file);

	return written == size;
}