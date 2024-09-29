#include "game_client_renderer.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <stdio.h>

#include "general/platformlib/platform.h"
#include "game_client.h"

char vertexShaderSource[] =
"#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"layout(location = 1) in vec3 aColor;\n"
"out vec3 vertexColor;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(aPos, 1.0);\n"
"    vertexColor = aColor;\n"
"}";


char fragmentShaderSource[] =
"#version 330 core\n"
"in vec3 vertexColor;\n"
"out vec4 FragColor;\n"
"void main() \n"
"{\n"
"    FragColor = vec4(vertexColor, 1.0); // Use vertex color with full alpha\n"
"}";



GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "Shader compilation error: %s\n", infoLog);
    }

    return shader;
}

// Function to create and compile the shader program
GLuint createShaderProgram() {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}


void renderer_init(struct game_client* game) {
    // Create and bind framebuffer
    glGenFramebuffers(1, &game->renderer.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, game->renderer.fbo);

    // Create texture for framebuffer
    glGenTextures(1, &game->renderer.texture);
    glBindTexture(GL_TEXTURE_2D, game->renderer.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, game->render_state.width, game->render_state.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Attach texture to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->renderer.texture, 0);

    // Check framebuffer status (optional but good practice)
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "Framebuffer not complete!\n");
    }

    // Setup shader
    game->renderer.default_shader_program = createShaderProgram();
    glUseProgram(game->renderer.default_shader_program); // Use shader program before getting uniform location
    game->renderer.buffer_size_uniform_loaction = glGetUniformLocation(game->renderer.default_shader_program, "uBufferSize");

    // Set clear color
    glClearColor(0.92f, 0.81f, 0.53f, 1.0f);

    // Generate and bind VAO/VBO
    glGenVertexArrays(1, &game->renderer.vao);
    glGenBuffers(1, &game->renderer.vbo);

    glBindVertexArray(game->renderer.vao);

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f, // Bottom left - Red
         0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f, // Bottom right - Green
         0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f  // Top middle - Blue
    };

    glBindBuffer(GL_ARRAY_BUFFER, game->renderer.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void client_renderer_adjust_size(struct game_client* game) {
    // Update texture size on resize
    glBindTexture(GL_TEXTURE_2D, game->renderer.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, game->render_state.width, game->render_state.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glBindFramebuffer(GL_FRAMEBUFFER, game->renderer.fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->renderer.texture, 0);

    // Update uniform for buffer size
    glUniform2f(game->renderer.buffer_size_uniform_loaction, game->render_state.width, game->render_state.height);
}

void client_render_world(struct game_client* game) {
    // Clear framebuffer and set the viewport
    glBindFramebuffer(GL_FRAMEBUFFER, game->renderer.fbo);
    glViewport(0, 0, game->render_state.width, game->render_state.height);
    glClear(GL_COLOR_BUFFER_BIT);

    // Define dynamic vertex data
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f, // Bottom left - Red
         0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f, // Bottom right - Green
         0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f  // Top middle - Blue
    };

    // Bind the vertex buffer and update it with the new data
    glBindBuffer(GL_ARRAY_BUFFER, game->renderer.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW); // Use GL_DYNAMIC_DRAW for dynamic data

    // Bind the VAO
    glBindVertexArray(game->renderer.vao);

    // Draw the triangle
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Read pixels from framebuffer
    glReadPixels(0, 0, game->render_state.width, game->render_state.height, GL_RGBA, GL_UNSIGNED_BYTE, game->render_state.pixels);

    // Unbind the VAO
    glBindVertexArray(0);
}


void renderer_exit(struct game_client* game) {
    // Unbind VAO and framebuffer before deletion
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Delete resources
    glDeleteVertexArrays(1, &game->renderer.vao);
    glDeleteBuffers(1, &game->renderer.vbo);
    glDeleteProgram(game->renderer.default_shader_program);
    glDeleteTextures(1, &game->renderer.texture);
    glDeleteFramebuffers(1, &game->renderer.fbo);
}
