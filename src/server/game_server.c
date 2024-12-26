#include "game_server.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>

#include "general/resource_manager.h"
#include "general/platformlib/platform.h"
#include "general/platformlib/networking.h"
#include "general/utils.h"
#include "general/logging.h"

#include "game/networking_packets/networking_packets.h"
#include "game/chat.h"

void new_game_server(struct game_server* game, char* resource_path) {
    game->resource_manager = new_resource_manager(resource_path);

    game->username_password_map = get_value_from_key(game->resource_manager, "passwords").ptr;

    struct key_value_map* settings_map = get_value_from_key(game->resource_manager, "settings").ptr;

    game->settings.max_clients = get_value_from_key(settings_map, "max_clients").i;
    game->settings.max_render_distance = get_value_from_key(settings_map, "max_render_distance").i;

    game->clients_connected_count = 0;
    game->clients_length = game->settings.max_clients;
    game->clients = malloc(game->settings.max_clients * sizeof(struct client_on_server));

    for (int i = 0; i < game->settings.max_clients; i++) {
        game->clients[i].client_handle = NULL;
        game->clients[i].next_packet_type = -1;
        for (int j = 0; j < 22 + 1; j++) game->clients[i].ip_address[j] = '\0';
        for (int j = 0; j < MAX_USERNAME_LENGTH + 1; j++) game->clients[i].username[j] = '\0';
    }

    int i = 0;
    for (; resource_path[i] != '\0'; i++);
    for (; resource_path[i] != '/' && i > 0; i--);
    if (resource_path[i] == '/') i++;

    for (int j = 0; j < i; j++) game->resource_folder_path[j] = resource_path[j];
    game->resource_folder_path[i] = '\0';
}


void client_auth_thread_function(struct game_server* game) {


    while (game->running) {
        void* client_handle;
        for (int i = 0; (client_handle = server_accept(game->server_handle)) != NULL && i < 10; i++) {

            int client_index = 0;
            if (game->clients_connected_count >= game->settings.max_clients) client_index = game->settings.max_clients;
            else for (; client_index < game->settings.max_clients && game->clients[client_index].client_handle != NULL; client_index++);
            

            int packet_type = -1;
            struct networking_packet_client_auth auth_packet;
            int recieved;

            if (receive_data(client_handle, &packet_type, sizeof(int), &game->running, 3000) == -2) {
                log_message(game->debug_log_file, "[CLIENT HANDLER] %s didnt send auth header", auth_packet.username);
                close_connection(client_handle);
                return;
            }

            if (packet_type != NETWORKING_PACKET_CLIENT_AUTH) {
                log_message(game->debug_log_file, "[CLIENT HANDLER] %s didnt send correct auth headers", auth_packet.username);
                close_connection(client_handle);
                return;
            }

            if (receive_data(client_handle, &auth_packet, sizeof(struct networking_packet_client_auth), &game->running, 3000) == -2) {
                log_message(game->debug_log_file, "[CLIENT HANDLER] %s didnt send auth packet", auth_packet.username);
                close_connection(client_handle);
                return;
            }

            char* password = get_value_from_key(game->username_password_map, auth_packet.username).ptr;

            if (password == NULL) {
                int packet_type = NETWORKING_PACKET_NOT_AUTHORIZED_TO_JOIN;
                send_data(client_handle, &packet_type, sizeof(int));
                close_connection(client_handle);
                log_message(game->debug_log_file, "[CLIENT HANDLER] %s not autherized to join", auth_packet.username);
            }
            else if (strcmp(auth_packet.password, password)) {
                int packet_type = NETWORKING_PACKET_INVALID_PASSWORD;
                send_data(client_handle, &packet_type, sizeof(int));
                close_connection(client_handle);
                log_message(game->debug_log_file, "[CLIENT HANDLER] %s incorrect password", auth_packet.username);
            }
            else if (client_index == game->settings.max_clients) {
                int packet_type = NETWORKING_PACKET_SERVER_FULL;
                send_data(client_handle, &packet_type, sizeof(int));
                close_connection(client_handle);
            }
            else {
                int packet_type = NETWORKING_PACKET_LOGGED_IN;
                send_data(client_handle, &packet_type, sizeof(int));
                packet_type = NETWORKING_PACKET_SERVER_SETTINGS;
                send_data(client_handle, &packet_type, sizeof(int));
                struct networking_packet_server_state server_state = {
                    game->settings.max_render_distance
                };

                for (int i = 0; i < game->clients_length; i++) if (game->clients[i].client_handle != NULL) if (strcmp(auth_packet.username, game->clients[i].username) == 0) { 
                    game->clients[i].logged_in_from_another_location = true; 
                    while (game->clients[i].client_handle != NULL) sleep_for_ms(1);
                    break;
                }

                send_data(client_handle, &server_state, sizeof(struct networking_packet_server_state));
           
                get_ip_address_and_port(client_handle, game->clients[client_index].ip_address, &game->clients[client_index].port);

                int j = 0;
                for (; auth_packet.username[j] != '\0'; j++) game->clients[client_index].username[j] = auth_packet.username[j];
                game->clients[client_index].username[j] = auth_packet.username[j];
                game->clients[client_index].logged_in_from_another_location = false;
                game->clients[client_index].just_logged_in = true;
                game->clients[client_index].client_handle = client_handle;
                game->clients_connected_count++;
                log_message(game->debug_log_file, "[CLIENT HANDLER] %s from %s:%u connected to the server", game->clients[client_index].username, game->clients[client_index].ip_address, game->clients[client_index].port);
            }
        }

        sleep_for_ms(100);
    }

    log_message(game->debug_log_file, "[CLIENT HANDLER] Stopped accepting new clients");

}

void disconnect_client(struct game_server* game, int client_index) {
    close_connection(game->clients[client_index].client_handle);
    game->clients[client_index].next_packet_type = -1;
    game->clients_connected_count--;
    game->clients[client_index].client_handle = NULL;

    char full_message[MAX_SERVER_MESSAGE_LENGTH + 1];
    struct game_chat_char message_buffer_chat[MAX_SERVER_MESSAGE_LENGTH + 1];
    sprintf(full_message, "%s left the game", game->clients[client_index].username);
    int message_buffer_chat_i = 0;
    for (; full_message[message_buffer_chat_i] != '\0'; message_buffer_chat_i++) {
        message_buffer_chat[message_buffer_chat_i] = (struct game_chat_char){ full_message[message_buffer_chat_i], 0, 0xffffff00 };
    }
    message_buffer_chat[message_buffer_chat_i] = (struct game_chat_char){ full_message[message_buffer_chat_i], 0, 0xffffff00 };
    int string_length = message_buffer_chat_i + 1;

    int packet_type = NETWORKING_PACKET_MESSAGE_FROM_SERVER;
    for (int j = 0; j < game->clients_length; j++) if (game->clients[j].client_handle != NULL) {
        send_data(game->clients[j].client_handle, &packet_type, sizeof(int));
        send_data(game->clients[j].client_handle, &string_length, sizeof(int));
        send_data(game->clients[j].client_handle, message_buffer_chat, sizeof(struct game_chat_char) * string_length);
    }

}

void server_log_init(struct game_server* game) {
    time_t raw_time = time(NULL);
    struct tm* time_info = localtime(&raw_time);

    char chat_log_path[1024];
    char* chat_log_path_rel = get_value_from_key(game->resource_manager, "chat_dump").s;
    int i = 0;
    if (chat_log_path_rel[0] != '/')for (; game->resource_folder_path[i] != '\0'; i++) chat_log_path[i] = game->resource_folder_path[i];
    int j = 0;
    for (; chat_log_path_rel[j] != '\0'; j++) chat_log_path[j + i] = chat_log_path_rel[j];

    chat_log_path[i + j] = digit_to_char((time_info->tm_mon + 1) / 10);
    chat_log_path[i + j + 1] = digit_to_char((time_info->tm_mon + 1) % 10);
    chat_log_path[i + j + 2] = digit_to_char(time_info->tm_mday / 10);
    chat_log_path[i + j + 3] = digit_to_char(time_info->tm_mday % 10);
    chat_log_path[i + j + 4] = digit_to_char((time_info->tm_year + 1900) / 1000);
    chat_log_path[i + j + 5] = digit_to_char(((time_info->tm_year + 1900) / 100) % 10);
    chat_log_path[i + j + 6] = digit_to_char(((time_info->tm_year + 1900) / 10) % 10);
    chat_log_path[i + j + 7] = digit_to_char((time_info->tm_year + 1900) % 10);
    chat_log_path[i + j + 8] = digit_to_char(time_info->tm_hour / 10);
    chat_log_path[i + j + 9] = digit_to_char(time_info->tm_hour % 10);
    chat_log_path[i + j + 10] = digit_to_char(time_info->tm_min / 10);
    chat_log_path[i + j + 11] = digit_to_char(time_info->tm_min % 10);
    chat_log_path[i + j + 12] = digit_to_char(time_info->tm_sec / 10);
    chat_log_path[i + j + 13] = digit_to_char(time_info->tm_sec % 10);
    chat_log_path[i + j + 14] = '.';
    chat_log_path[i + j + 15] = 'l';
    chat_log_path[i + j + 16] = 'o';
    chat_log_path[i + j + 17] = 'g';
    chat_log_path[i + j + 18] = '\0';

    game->chat_log_file = fopen(chat_log_path, "w");

    char debug_log_path[1024];
    char* debug_log_path_rel = get_value_from_key(game->resource_manager, "debug_dump").s;
    i = 0;
    if (debug_log_path_rel[0] != '/') for (; game->resource_folder_path[i] != '\0'; i++) debug_log_path[i] = game->resource_folder_path[i];
    j = 0;
    for (; debug_log_path_rel[j] != '\0'; j++) debug_log_path[j + i] = debug_log_path_rel[j];

    debug_log_path[i + j] = digit_to_char((time_info->tm_mon + 1) / 10);
    debug_log_path[i + j + 1] = digit_to_char((time_info->tm_mon + 1) % 10);
    debug_log_path[i + j + 2] = digit_to_char(time_info->tm_mday / 10);
    debug_log_path[i + j + 3] = digit_to_char(time_info->tm_mday % 10);
    debug_log_path[i + j + 4] = digit_to_char((time_info->tm_year + 1900) / 1000);
    debug_log_path[i + j + 5] = digit_to_char(((time_info->tm_year + 1900) / 100) % 10);
    debug_log_path[i + j + 6] = digit_to_char(((time_info->tm_year + 1900) / 10) % 10);
    debug_log_path[i + j + 7] = digit_to_char((time_info->tm_year + 1900) % 10);
    debug_log_path[i + j + 8] = digit_to_char(time_info->tm_hour / 10);
    debug_log_path[i + j + 9] = digit_to_char(time_info->tm_hour % 10);
    debug_log_path[i + j + 10] = digit_to_char(time_info->tm_min / 10);
    debug_log_path[i + j + 11] = digit_to_char(time_info->tm_min % 10);
    debug_log_path[i + j + 12] = digit_to_char(time_info->tm_sec / 10);
    debug_log_path[i + j + 13] = digit_to_char(time_info->tm_sec % 10);
    debug_log_path[i + j + 14] = '.';
    debug_log_path[i + j + 15] = 'l';
    debug_log_path[i + j + 16] = 'o';
    debug_log_path[i + j + 17] = 'g';
    debug_log_path[i + j + 18] = '\0';

    game->debug_log_file = fopen(debug_log_path, "w");
}

void run_game_server(struct game_server* game) {

    server_log_init(game);

    log_message(game->debug_log_file, "[SERVER] Server starting ...");
    show_console_window();



    game->server_handle = server_init(8080);
    if (!game->server_handle) {
        log_message(game->debug_log_file, "[SERVER] Failed to initialize server");
        return;
    }
    log_message(game->debug_log_file, "[SERVER] Server listening on port 8080");

    game->running = true;

    //create_thread();

    void* client_auth_thread = create_thread((void (*) (void*))client_auth_thread_function, game);

    while (game->running) {
        

        

        for (int i = 0; i < game->clients_length; i++) if (game->clients[i].client_handle != NULL) {

            if (game->clients[i].logged_in_from_another_location) {
                int kick_packet_type = NETWORKING_PACKET_KICK;
                struct networking_packet_kick kick_packet = { "You logged in from another location" };
                send_data(game->clients[i].client_handle, &kick_packet_type, sizeof(int));
                send_data(game->clients[i].client_handle, &kick_packet, sizeof(struct networking_packet_kick));
                disconnect_client(game, i);
                log_message(game->chat_log_file, "[SERVER] %s kicked because they logged in from another location", game->clients[i].username);

                continue;
            }
            else if (game->clients[i].just_logged_in) {

                game->clients[i].just_logged_in = false;

                char full_message[MAX_SERVER_MESSAGE_LENGTH + 1];
                struct game_chat_char message_buffer_chat[MAX_SERVER_MESSAGE_LENGTH + 1];
                sprintf(full_message, "%s joined the game", game->clients[i].username);
                int message_buffer_chat_i = 0;
                for (; full_message[message_buffer_chat_i] != '\0'; message_buffer_chat_i++) {
                    message_buffer_chat[message_buffer_chat_i] = (struct game_chat_char){ full_message[message_buffer_chat_i], 0, 0xffffff00 };
                }
                message_buffer_chat[message_buffer_chat_i] = (struct game_chat_char){ full_message[message_buffer_chat_i], 0, 0xffffff00 };
                int string_length = message_buffer_chat_i + 1;

                int packet_type = NETWORKING_PACKET_MESSAGE_FROM_SERVER;
                for (int j = 0; j < game->clients_length; j++) if (game->clients[j].client_handle != NULL) {
                    send_data(game->clients[j].client_handle, &packet_type, sizeof(int));
                    send_data(game->clients[j].client_handle, &string_length, sizeof(int));
                    send_data(game->clients[j].client_handle, message_buffer_chat, sizeof(struct game_chat_char) * string_length);
                }
            }

            for (int packet_count = 0; packet_count < 5; packet_count++) {
                if (game->clients[i].next_packet_type == -1) receive_data(game->clients[i].client_handle, &game->clients[i].next_packet_type, sizeof(int), NULL, 0);

                if (game->clients[i].next_packet_type == -1) break;

                switch (game->clients[i].next_packet_type) {


                case NETWORKING_PACKET_DISCONNECT: {
                    game->clients[i].next_packet_type = -1;
                    log_message(game->chat_log_file, "[SERVER] %s disconnected", game->clients[i].username);
                    disconnect_client(game, i);
                    goto _connection_closed;
                } break;

                case NETWORKING_PACKET_MESSAGE_TO_SERVER: {
                    char message_buffer[MAX_CHAT_MESSAGE_LENGTH + 1];
                    if (receive_data(game->clients[i].client_handle, message_buffer, sizeof(struct networking_packet_chat_message_to_server), NULL, 0) > 0) {

                        if (message_buffer[0] == '/') {

                            int next_word = 1;
                            for (; message_buffer[next_word] == ' '; next_word++);

                            if (!strcmp(&message_buffer[next_word], "stop")) {
                                game->running = false;
                            }
                            else if (!strcmp(&message_buffer[next_word], "help")) {
                                char full_message[MAX_SERVER_MESSAGE_LENGTH + 1];
                                struct game_chat_char message_buffer_chat[MAX_SERVER_MESSAGE_LENGTH + 1];

                                sprintf(full_message, "Commands:\n  help: list all commands\n  stop: stop the server\n  command1: action1\n  command2: action2");

                                int message_buffer_chat_i = 0;
                                for (; full_message[message_buffer_chat_i] != '\0'; message_buffer_chat_i++) {
                                    message_buffer_chat[message_buffer_chat_i] = (struct game_chat_char){ full_message[message_buffer_chat_i], 0, 0xffffffff };
                                }
                                message_buffer_chat[message_buffer_chat_i] = (struct game_chat_char){ full_message[message_buffer_chat_i], 0, 0xffffffff };

                                int string_length = message_buffer_chat_i + 1;

                                int packet_type = NETWORKING_PACKET_MESSAGE_FROM_SERVER;

                                send_data(game->clients[i].client_handle, &packet_type, sizeof(int));
                                send_data(game->clients[i].client_handle, &string_length, sizeof(int));
                                send_data(game->clients[i].client_handle, message_buffer_chat, sizeof(struct game_chat_char) * string_length);
                            }
                            else {
                                char full_message[MAX_SERVER_MESSAGE_LENGTH + 1];
                                struct game_chat_char message_buffer_chat[MAX_SERVER_MESSAGE_LENGTH + 1];

                                sprintf(full_message, "'%s' is not recognised as a command", message_buffer);

                                int message_buffer_chat_i = 0;
                                for (; full_message[message_buffer_chat_i] != '\0'; message_buffer_chat_i++) {
                                    message_buffer_chat[message_buffer_chat_i] = (struct game_chat_char){ full_message[message_buffer_chat_i], 0, 0xffD61A1A };
                                }
                                message_buffer_chat[message_buffer_chat_i] = (struct game_chat_char){ full_message[message_buffer_chat_i], 0, 0xffD61A1A };

                                int string_length = message_buffer_chat_i + 1;

                                int packet_type = NETWORKING_PACKET_MESSAGE_FROM_SERVER;

                                send_data(game->clients[i].client_handle, &packet_type, sizeof(int));
                                send_data(game->clients[i].client_handle, &string_length, sizeof(int));
                                send_data(game->clients[i].client_handle, message_buffer_chat, sizeof(struct game_chat_char) * string_length);
                                
                            }
                        }

                        else {
                            char full_message[MAX_SERVER_MESSAGE_LENGTH + 1];
                            struct game_chat_char message_buffer_chat[MAX_SERVER_MESSAGE_LENGTH + 1];

                            sprintf(full_message, "<%s> %s", game->clients[i].username, message_buffer);

                            int message_buffer_chat_i = 0;
                            for (; full_message[message_buffer_chat_i] != '\0'; message_buffer_chat_i++) {
                                message_buffer_chat[message_buffer_chat_i] = (struct game_chat_char){ full_message[message_buffer_chat_i], 0, 0xffffffff };
                            }
                            message_buffer_chat[message_buffer_chat_i] = (struct game_chat_char){ full_message[message_buffer_chat_i], 0, 0xffffffff };

                            int string_length = message_buffer_chat_i + 1;

                            int packet_type = NETWORKING_PACKET_MESSAGE_FROM_SERVER;

                            for (int j = 0; j < game->clients_length; j++) if (game->clients[j].client_handle != NULL) {
                                send_data(game->clients[j].client_handle, &packet_type, sizeof(int));
                                send_data(game->clients[j].client_handle, &string_length, sizeof(int));
                                send_data(game->clients[j].client_handle, message_buffer_chat, sizeof(struct game_chat_char) * string_length);
                            }

                            log_message(game->chat_log_file, full_message);
                        }
                        
                        game->clients[i].next_packet_type = -1;
                    }
                } break;

                case 2: {

                } break;

                default: {
                    game->clients[i].next_packet_type = -1;
                    int kick_packet_type = NETWORKING_PACKET_KICK;
                    struct networking_packet_kick kick_packet = { "Invalid packets" };
                    send_data(game->clients[i].client_handle, &kick_packet_type, sizeof(int));
                    send_data(game->clients[i].client_handle, &kick_packet, sizeof(struct networking_packet_kick));
                    disconnect_client(game, i);
                    log_message(game->chat_log_file, "[SERVER] %s kicked due to invalid packet", game->clients[i].username);

                } break;

                }

            }


        _connection_closed:
            1 == 1;
        }



        sleep_for_ms(10);
        if (get_key_state(KEY_ALT_L)) game->running = false;
    }

    join_thread(client_auth_thread);

    int kick_packet_type = NETWORKING_PACKET_KICK;
    struct networking_packet_kick kick_packet = { "Server stopped" };

    for (int i = 0; i < game->clients_length; i++) if (game->clients[i].client_handle != NULL) { 
        send_data(game->clients[i].client_handle, &kick_packet_type, sizeof(int));
        send_data(game->clients[i].client_handle, &kick_packet, sizeof(struct networking_packet_kick));
        close_connection(game->clients[i].client_handle); 
        log_message(game->chat_log_file, "[SERVER] %s Disconnected", game->clients[i].username);
    }

    log_message(game->debug_log_file, "[SERVER] Server closed");
    server_close(game->server_handle);

    fclose(game->chat_log_file);
    fclose(game->debug_log_file);

}

void delete_game_server(struct game_server* game) {
    destroy_resource_manager(game->resource_manager);
}