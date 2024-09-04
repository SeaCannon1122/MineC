#pragma once

enum buffer_properties {
	BUFFER_IN = 0b01,
	BUFFER_OUT = 0b10,
};

void* create_kernel(char* src);
void run_kernel(void* kernel, char dimension, int dim_x, int dim_y, int dim_z, ...);
void destroy_kernel(void* src);