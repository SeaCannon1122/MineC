#pragma once

#define GLEW_STATIC

#include <stdbool.h>

#include <GL/glew.h>
#include <GL/gl.h>

#if defined(_WIN32)
#define ASSERT(x) if(!(x)) __debugbreak();
#elif defined(__linux__)
#include <signal.h>
#define ASSERT(x) if(!(x)) 1==1;//raise(SIGTRAP);
#endif;


const static char* getGLErrorString(GLenum error) {
    switch (error) {
    case GL_NO_ERROR:         return "No error";
    case GL_INVALID_ENUM:     return "Invalid enum";
    case GL_INVALID_VALUE:    return "Invalid value";
    case GL_INVALID_OPERATION: return "Invalid operation";
    case GL_STACK_OVERFLOW:   return "Stack overflow";
    case GL_STACK_UNDERFLOW:  return "Stack underflow";
    case GL_OUT_OF_MEMORY:    return "Out of memory";
    default:                  return "Unknown error";
    }
}

#define GLCall(call) do { \
    call; \
    GLenum err; \
    while ((err = glGetError()) != GL_NO_ERROR) { \
        printf("OpenGL error in %s at %s:%d: %s (code: %d)\n", \
                #call, __FILE__, __LINE__, getGLErrorString(err), err); \
       __debugbreak(); \
    } \
} while(0)

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);


int opengl_init(int version_major, int version_minor);

void opengl_exit();

GLuint compileShader(GLenum type, const char* source);

GLuint createShaderProgram(char* vertex_shader_source, char* fragment_shader_source);

