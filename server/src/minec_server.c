#include "minec_server.h"

uint32_t game_server_run(struct minec_server* game, uint8_t* resource_path) {

    resources_create(game, resource_path);
    settings_load(game);

    networker_start(game);

    while (1) {

        printf("tick\n");

        sleep_for_ms(20);
    }

    networker_stop(game);

    resources_destroy(game);
    
    return 0;
}