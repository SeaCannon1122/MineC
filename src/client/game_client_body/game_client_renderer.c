#include "game_client_renderer.h"


void renderer_init(struct game_client* game) {


    

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


}

void client_renderer_adjust_size(struct game_client* game) {

}

void client_render_world(struct game_client* game) {

}


void renderer_exit(struct game_client* game) {

}
