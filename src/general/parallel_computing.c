#include "parallel_computing.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <CL/cl.h>

#define MAX_KERNEL_ARGS 128

enum arg_types {
    ARG_CHAR = 0,
    ARG_SHORT = 1,
    ARG_INT = 2,
    ARG_LONG = 3,
    ARG_FLOAT = 4,
    ARG_POINTER = 5
};

//Assumed that if arg is pointer, next arg is int length and next again is buffer properties

struct program_resources {
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;
    cl_device_id device;

    int arg_count;
    char arg_types[MAX_KERNEL_ARGS];
};

#define CL_CALL(call)\
do {\
cl_int err = (call);\
if (err != CL_SUCCESS) {\
fprintf(stderr, "OpenCL error in\n     %s\n  at %s:%d: %d\n", #call, __FILE__, __LINE__, err);\
exit(1);\
}\
} while(0)

#define CL_OBJECT_CALL(type, object, call)\
type object;\
do {\
cl_int err;\
object = (call);\
if (err != CL_SUCCESS) {\
fprintf(stderr, "OpenCL error in \n     %s \n  at %s:%d: %d\n^\n",#call, __FILE__, __LINE__, err);\
exit(1);\
}\
} while(0)


void* create_kernel(char* src) {
    struct program_resources* resources = malloc(sizeof(struct program_resources));
    if (resources == NULL) return NULL;
    resources->arg_count = 0;

    int i = 0;
    for (; src[i] != '('; i++);
    for (; src[i] == ' '; i++);
    for (; src[i] != ')'; resources->arg_count++) {

        if (src[i] == 'c' && src[i + 1] == 'h' && src[i + 2] == 'a' && src[i + 3] == 'r') {
            i += 4;
            resources->arg_types[resources->arg_count] = ARG_CHAR;
        }
        else if (src[i] == 's' && src[i + 1] == 'h' && src[i + 2] == 'o' && src[i + 3] == 'r' && src[i + 4] == 't') {
            i += 5;
            resources->arg_types[resources->arg_count] = ARG_SHORT;
        }
        else if (src[i] == 'i' && src[i + 1] == 'n' && src[i + 2] == 't') {
            i += 3;
            resources->arg_types[resources->arg_count] = ARG_INT;
        }
        else if (src[i] == 'l' && src[i + 1] == 'o' && src[i + 2] == 'n' && src[i + 3] == 'g') {
            i += 4;
            resources->arg_types[resources->arg_count] = ARG_LONG;
        }
        else if (src[i] == 'f' && src[i + 1] == 'l' && src[i + 2] == 'o' && src[i + 3] == 'a' && src[i + 4] == 't') {
            i += 5;
            resources->arg_types[resources->arg_count] = ARG_FLOAT;
        }
        else {
            for (; src[i] != '*'; i++);
            i++;
            resources->arg_types[resources->arg_count] = ARG_POINTER;
        }

        for (; src[i] == ' '; i++);
        for (; src[i] != ' ' && src[i] != ')'; i++);
        for (; src[i] == ' '; i++);
    }

    i = 0;
    for (; src[i] != 'd'; i++);
    i++;
    for (; src[i] == ' '; i++);

    int kernel_name_start = i;

    for (; src[i] != ' ' && src[i] != '('; i++);

    char* kernel_name = malloc(i - (long long)kernel_name_start + 1);
    if (kernel_name == NULL) {
        free(resources);
        return NULL;
    }

    cl_platform_id platform;
    CL_CALL(clGetPlatformIDs(1, &platform, NULL));

    CL_CALL(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &resources->device, NULL));

    CL_OBJECT_CALL(, resources->context, clCreateContext(NULL, 1, &resources->device, NULL, NULL, &err));

    CL_OBJECT_CALL(, resources->queue, clCreateCommandQueue(resources->context, resources->device, 0, &err));

    CL_OBJECT_CALL(, resources->program, clCreateProgramWithSource(resources->context, 1, (const char**)&src, NULL, &err));

    CL_CALL(clBuildProgram(resources->program, 1, &resources->device, NULL, NULL, NULL));
    
    for (int j = kernel_name_start; j < i; j++) kernel_name[j - kernel_name_start] = src[j];
    kernel_name[i - kernel_name_start] = '\0';

    CL_OBJECT_CALL(, resources->kernel, clCreateKernel(resources->program, kernel_name, &err));

    free(kernel_name);
    return resources;
}

void run_kernel(void* kernel, char dimension, int dim_x, int dim_y, int dim_z, ...) {
    struct program_resources* resources = (struct program_resources*)kernel;

    cl_mem mem_objects[64];
    void* mem_pointers[64];
    int mem_object_sizes[64];
    int buffer_properties[64];
    int mem_objects_count = 0;

    va_list args;
    va_start(args, dim_z);

    for (int i = 0; i < resources->arg_count; i++) {
        
        switch (resources->arg_types[i]) {
            
        case ARG_CHAR: {
            char char_arg = va_arg(args, int);
            CL_CALL(clSetKernelArg(resources->kernel, i, sizeof(char), &char_arg));
            break;  
        }
            
        case ARG_SHORT: {
            short short_arg = va_arg(args, int);
            CL_CALL(clSetKernelArg(resources->kernel, i, sizeof(short), &short_arg));
            break;
        }

        case ARG_INT: {
            int int_arg = va_arg(args, int);
            CL_CALL(clSetKernelArg(resources->kernel, i, sizeof(int), &int_arg));
            break;
        }

        case ARG_LONG: {
            long long long_arg = va_arg(args, long long);
            CL_CALL(clSetKernelArg(resources->kernel, i, sizeof(long long), &long_arg));
            break;
        }

        case ARG_FLOAT: {
            float float_arg = va_arg(args, double);
            CL_CALL(clSetKernelArg(resources->kernel, i, sizeof(float), &float_arg));
            break;
        }

        case ARG_POINTER: {
            void* pointer_arg = va_arg(args, void*);
            int length_arg = va_arg(args, int);
            int buffer_properties_arg = va_arg(args, int);

            buffer_properties[mem_objects_count] = buffer_properties_arg;
            CL_OBJECT_CALL(, mem_objects[mem_objects_count], clCreateBuffer(resources->context, CL_MEM_READ_WRITE, length_arg, NULL, &err));

            if(buffer_properties_arg & BUFFER_COPY) CL_CALL(clEnqueueWriteBuffer(resources->queue, mem_objects[mem_objects_count], CL_TRUE, 0, length_arg, pointer_arg, 0, NULL, NULL));

            mem_pointers[mem_objects_count] = pointer_arg;
            mem_object_sizes[mem_objects_count] = length_arg;
            
            CL_CALL(clSetKernelArg(resources->kernel, i, sizeof(cl_mem), &mem_objects[mem_objects_count]));
            mem_objects_count++;

            break;
        }

        }
    }

    size_t globalItemSize[3] = { dim_x, dim_y, dim_z};
    CL_CALL(clEnqueueNDRangeKernel(resources->queue, resources->kernel, dimension, NULL, globalItemSize, NULL, 0, NULL, NULL));

    CL_CALL(clFlush(resources->queue));
    CL_CALL(clFinish(resources->queue));
    
    for (int i = 0; i < mem_objects_count; i++) {
        
        if (buffer_properties[i] & BUFFER_READ) CL_CALL(clEnqueueReadBuffer(resources->queue, mem_objects[i], CL_TRUE, 0, mem_object_sizes[i], mem_pointers[i], 0, NULL, NULL));
        CL_CALL(clReleaseMemObject(mem_objects[i]));
    }
        
    return;
}

void destroy_kernel(void* kernel) {
    struct program_resources* resources = (struct program_resources*)kernel;

    clReleaseKernel(resources->kernel);
    clReleaseProgram(resources->program);
    clReleaseCommandQueue(resources->queue);
    clReleaseContext(resources->context);
}