#pragma once

#include "stddef.h"

void parallel_computing_init();
void parallel_computing_exit();

void* create_kernel(char* src);
void run_kernel(void* kernel, char dimension, int dim_x, int dim_y, int dim_z, ...);
void destroy_kernel(void* kernel);

void* create_device_buffer(size_t size);
void write_to_device_buffer(void* device_buffer, void* host_buffer, size_t size, size_t offset);
void read_from_device_buffer(void* device_buffer, void* host_buffer, size_t size, size_t offset);
void destroy_device_buffer(void* memory_buffer);

void wait_for_parallel_execution_completion();