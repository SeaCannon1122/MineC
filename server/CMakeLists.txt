cmake_minimum_required(VERSION 3.15)

project(MineC_Server)

set(SERVER_SOURCE_FILES
    src/server/main.c
    src/server/minec_server.c
    src/server/resources/resources.c
    src/server/networker/networker.c
    src/server/settings/settings.c

    src/general/key_value.c
    src/general/resource_manager.c
    src/general/platformlib/platform/platform.c
    src/general/platformlib/networking/networking.c
)