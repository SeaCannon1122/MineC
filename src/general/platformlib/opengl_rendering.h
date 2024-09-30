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


#define GLCall(x) GLClearError();\
x;\
ASSERT(GLLogCall(#x, __FILE__, __LINE__))

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);


int opengl_init();

void opengl_exit();

GLuint compileShader(GLenum type, const char* source);

GLuint createShaderProgram(char* vertex_shader_source, char* fragment_shader_source);

