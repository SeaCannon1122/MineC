#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>

#include "general/platformlib/platform.h"
#include "general/platformlib/opengl_rendering.h"
#include "test/glframebuffer.h"
#include "test/shader.h"

const char* vertexShaderSource =
"#version 330 core\n"
"layout(location = 0) in vec2 position;\n"
"out vec4 fragColor;\n"
"void main() {\n"
"    const int screenWidth = 800;\n"
"    const int screenHeight = 600;\n"
"    float ndcX = (position.x / screenWidth) * 2.0 - 1.0;\n"
"    float ndcY = 1.0 - (position.y / screenHeight) * 2.0;\n"
"    gl_Position = vec4(ndcX, ndcY, 0.0, 1.0);\n"
"    fragColor = vec4(1.0, 0.0, 0.0, 1.0); // Red color\n"  // Set the color to red
"}\n";

// Fragment Shader Source
const char* fragmentShaderSource =
"#version 330 core\n"
"in vec4 fragColor;\n"
"out vec4 color;\n"
"void main() {\n"
"    color = fragColor;\n"
"}\n";

// Main function
int testing_main() {

    show_console_window();
    printf("Running OpenGL %s\n", glGetString(GL_VERSION));

    struct frame_buffer fb;

    int width = 800;
    int height = 600;

    frame_buffer_new(&fb, width, height);

    int window = window_create(100, 100, width, height, "NAME");

    width = window_get_width(window);
    height = window_get_width(window);

    unsigned int* pixels = malloc(sizeof(unsigned int) * height * width);

    frame_buffer_bind(&fb);


    GLuint shaderProgram = shader_create_program(2, vertexShaderSource, GL_VERTEX_SHADER, fragmentShaderSource, GL_FRAGMENT_SHADER);

    // Example pixel coordinates
    int pixelCoords[] = {
        100, 150,
        200, 250,
        300, 350
    };

    // Setup vertex buffer
    GLuint vbo, vao;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao); // Bind the VAO

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pixelCoords), pixelCoords, GL_STATIC_DRAW);

    // Specify the layout for the pixel coordinates
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(int), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind the VBO
    glBindVertexArray(0); // Unbind the VAO
    GLCall(glClearColor(0.92f, 0.81f, 0.53f, 1.0f));
    GLCall(glViewport(0, 0, width, height));

    while (window_is_active(window)) {

        int new_width = window_get_width(window);
        int new_height = window_get_height(window);

        if (width != new_width || height != new_height) {
            height = new_height;
            width = new_width;
            free(pixels);
            pixels = malloc(sizeof(unsigned int) * height * width);

            GLCall(glViewport(0, 0, width, height));
            frame_buffer_set_size(&fb, width, height);
        }


        glClear(GL_COLOR_BUFFER_BIT);

        // Use the shader program
        glUseProgram(shaderProgram);

        glBindVertexArray(vao); // Bind the VAO
        glDrawArrays(GL_TRIANGLES, 0, sizeof(pixelCoords) / (2 * sizeof(int))); // Divide by 2 for number of points
        glBindVertexArray(0); // Unbind the VAO

        GLCall(glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels));

        window_draw(window, pixels, width, height, 1);

        sleep_for_ms(5);
    }

    // Cleanup
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shaderProgram);

    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));



    frame_buffer_destroy(&fb);

    window_destroy(window);

    return 0;
}
