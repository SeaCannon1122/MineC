#include "shader.h"

#include <stdarg.h>
#include <stdio.h>

#include "general/platformlib/opengl_rendering.h"

GLuint compile_shader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    GLCall(glShaderSource(shader, 1, &source, NULL));
    GLCall(glCompileShader(shader));

    GLint success;
    GLCall(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("Shader compilation error: %s\n", infoLog);
    }

    return shader;
}

GLuint shader_create_program(int shader_count, ...) {

    GLuint shaders[128];

	va_list args;

	va_start(args, shader_count);

	GLuint shader_program = glCreateProgram();

	for (int i = 0; i < shader_count; i++) {
	    
        void* source = va_arg(args, void*);
        GLenum type = va_arg(args, GLenum);

        shaders[i] = compile_shader(type, source);
        GLCall(glAttachShader(shader_program, shaders[i]));
	}

    GLCall(glLinkProgram(shader_program));
    
    GLint success;
    GLchar infoLog[512];

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
        printf("ERROR::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }

    for (int i = 0; i < shader_count; i++) {
        GLCall(glDeleteShader(shaders[i]));
    }
    
    return shader_program;
}