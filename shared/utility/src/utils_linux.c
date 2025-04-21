#include "platform_utils.h"

#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <malloc.h>

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