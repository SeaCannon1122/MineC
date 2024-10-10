#include "game_client_renderer.h"

#define GLEW_STATIC

#include <GL/glew.h>
#include <GL/gl.h>
#include <stdio.h>

#include "general/platformlib/platform.h"
#include "general/platformlib/opengl_rendering.h"
#include "game_client.h"
#include "general/argb_image.h"

char vertexShaderSource[] =
"#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"layout(location = 1) in vec3 aColor;\n"
"out vec3 vertexColor;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(aPos.x, -aPos.y, aPos.z, 1.0);\n"
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


void renderer_init(struct game_client* game) {

    printf("Running OpenGL %s\n", glGetString(GL_VERSION));

    // Create and bind framebuffer
    GLCall(glGenFramebuffers(1, &game->renderer.fbo));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, game->renderer.fbo));

    // Create texture for framebuffer
    GLCall(glGenTextures(1, &game->renderer.texture));
    GLCall(glBindTexture(GL_TEXTURE_2D, game->renderer.texture));
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, game->render_state.width, game->render_state.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    // Attach texture to framebuffer
    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->renderer.texture, 0));

    // Check framebuffer status (optional but good practice)
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "Framebuffer not complete!\n");
    }


    

    // Set clear color
    GLCall(glClearColor(0.92f, 0.81f, 0.53f, 1.0f));


    float vertices[] = {
        0.5f,  0.5f, 0.0f,  1.0f, 0.0f, // Top Right
         0.5f, -0.5f, 0.0f,  1.0f, 1.0f, // Bottom Right
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, // Bottom Left
        -0.5f,  0.5f, 0.0f,  0.0f, 0.0f  // Top Left 
    };

    unsigned int indecies[] = {
        0, 1, 3, // First triangle
        1, 2, 3  // Second triangle
    };

    GLCall(glGenVertexArrays(1, &game->renderer.vao));
    GLCall(glBindVertexArray(game->renderer.vao));

    GLCall(glGenBuffers(1, &game->renderer.vbo));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, game->renderer.vbo));
    GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    GLCall(glGenBuffers(1, &game->renderer.ibo));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, game->renderer.ibo));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indecies), indecies, GL_STATIC_DRAW));


    //setup_shader
    char* vs = get_value_from_key(game->resource_manager, "default_vertex").ptr;
    char* fs = get_value_from_key(game->resource_manager, "default_fragment").ptr;

    game->renderer.shaders.default_shader_program = createShaderProgram(vs, fs);
    GLCall(glUseProgram(game->renderer.shaders.default_shader_program));


    struct argb_image* test_texture = get_value_from_key(game->resource_manager, "dirt_texture").ptr;
    glGenTextures(1, &game->renderer.test_texture);
    glActiveTexture(GL_TEXTURE);
    glBindTexture(GL_TEXTURE_2D, game->renderer.test_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, test_texture->width, test_texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, test_texture->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);


    GLCall(glBindVertexArray(0));
    GLCall(glUseProgram(0));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

    glBindTexture(GL_TEXTURE_2D, 0);

    //set viewport
    GLCall(glViewport(0, 0, game->render_state.width, game->render_state.height));
}

void client_renderer_adjust_size(struct game_client* game) {
    // Update texture size on resize
    GLCall(glBindTexture(GL_TEXTURE_2D, game->renderer.texture));
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, game->render_state.width, game->render_state.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));

    GLCall(glViewport(0, 0, game->render_state.width, game->render_state.height));
}

void client_render_world(struct game_client* game) {

    GLCall(glClear(GL_COLOR_BUFFER_BIT));

    GLCall(glUseProgram(game->renderer.shaders.default_shader_program));
    
    glBindTexture(GL_TEXTURE_2D, game->renderer.test_texture);

    GLCall(glBindVertexArray(game->renderer.vao));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, game->renderer.ibo));

    GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL));

    GLCall(glReadPixels(0, 0, game->render_state.width, game->render_state.height, GL_RGBA, GL_UNSIGNED_BYTE, game->render_state.pixels));

}


void renderer_exit(struct game_client* game) {
    // Unbind VAO and framebuffer before deletion
    GLCall(glBindVertexArray(0));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    // Delete resources
    GLCall(glDeleteVertexArrays(1, &game->renderer.vao));
    GLCall(glDeleteBuffers(1, &game->renderer.vbo));
    GLCall(glDeleteProgram(game->renderer.shaders.default_shader_program));
    GLCall(glDeleteTextures(1, &game->renderer.texture));
    GLCall(glDeleteFramebuffers(1, &game->renderer.fbo));
}
