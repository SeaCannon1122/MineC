#pragma once

void* create_kernel(char* src);
void run_kernel(void* kernel, char dimension, int dim_x, int dim_y, int dim_z, ...);
void destroy_kernel(void* src);