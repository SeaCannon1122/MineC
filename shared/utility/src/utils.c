#include <utils.h>

#include <stdio.h>
#include <string.h>

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

#include <dlfcn.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

void* dynamic_library_load(uint8_t* library_name, bool name_explicit)
{

	uint8_t* load_library_name = library_name;

	if (name_explicit == false)
	{
		size_t library_name_length = strlen(library_name);
		load_library_name = malloc(sizeof("lib") - 1 + library_name_length + sizeof(".so"));

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

float get_time()
{
	LARGE_INTEGER frequency, start;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&start);

	return (float)1000 * ((float)start.QuadPart / (float)frequency.QuadPart);
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

float get_time()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (float)tv.tv_sec * 1000. + (float)tv.tv_usec / 1000.;
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

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif



// Utility: check if a name is "." or ".."
int is_dot_or_dotdot(const char* name) {
	return (strcmp(name, ".") == 0 || strcmp(name, "..") == 0);
}

// Shared result construction helper
char* join_names(const char** names, int count) {
	if (count == 0) return strdup(""); // empty string

	// Calculate total size
	size_t total_len = 0;
	for (int i = 0; i < count; ++i)
		total_len += strlen(names[i]) + 1; // +1 for '/'

	char* result = malloc(total_len + 1); // +1 for null terminator
	if (!result) return NULL;

	result[0] = '\0';
	for (int i = 0; i < count; ++i) {
		if (i > 0) strcat(result, "/");
		strcat(result, names[i]);
	}
	return result;
}

char* list_subdirectories(const char* path) {
#ifdef _WIN32
	char search_path[MAX_PATH];
	snprintf(search_path, sizeof(search_path), "%s/*", path);



	WIN32_FIND_DATAA ffd;
	HANDLE hFind = FindFirstFileA(search_path, &ffd);
	if (hFind == INVALID_HANDLE_VALUE) return NULL;

	const char* names[1024]; // max 1024 dirs for simplicity
	int count = 0;

	do {
		if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !is_dot_or_dotdot(ffd.cFileName)) {
			names[count++] = _strdup(ffd.cFileName);
		}
	} while (FindNextFileA(hFind, &ffd) && count < 1024);

	FindClose(hFind);
	char* result = join_names(names, count);

	for (int i = 0; i < count; ++i) free((void*)names[i]);
	return result;

#else
	DIR* dir = opendir(path);
	if (!dir) return NULL;

	const char* names[1024];
	int count = 0;
	struct dirent* entry;

	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type == DT_DIR && !is_dot_or_dotdot(entry->d_name)) {
			names[count++] = strdup(entry->d_name);
		}
	}
	closedir(dir);
	char* result = join_names(names, count);

	for (int i = 0; i < count; ++i) free((void*)names[i]);
	return result;
#endif
}

char* list_files(const char* path) {
#ifdef _WIN32
	char search_path[MAX_PATH];
	snprintf(search_path, sizeof(search_path), "%s/*", path);

	WIN32_FIND_DATAA ffd;
	HANDLE hFind = FindFirstFileA(search_path, &ffd);
	if (hFind == INVALID_HANDLE_VALUE) return NULL;

	const char* names[1024];
	int count = 0;

	do {
		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			names[count++] = _strdup(ffd.cFileName);
		}
	} while (FindNextFileA(hFind, &ffd) && count < 1024);

	FindClose(hFind);
	char* result = join_names(names, count);

	for (int i = 0; i < count; ++i) free((void*)names[i]);
	return result;

#else
	DIR* dir = opendir(path);
	if (!dir) return NULL;

	const char* names[1024];
	int count = 0;
	struct dirent* entry;

	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type == DT_REG) {
			names[count++] = strdup(entry->d_name);
		}
	}
	closedir(dir);
	char* result = join_names(names, count);

	for (int i = 0; i < count; ++i) free((void*)names[i]);
	return result;
#endif
}