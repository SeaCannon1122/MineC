#ifndef MINEC_CLIENT_LOGGING
#define MINEC_CLIENT_LOGGING

#include <stdint.h>
#include <stdarg.h>

void minec_client_log(struct minec_client* client, uint8_t* message, ...);

void minec_client_log_error(struct minec_client* client, uint8_t* message, ...);
void _minec_client_log_debug_error(struct minec_client* client, uint8_t* function, uint8_t* file, uint32_t line, uint8_t* message, ...);
#define minec_client_log_debug_error(client, message, ...) _minec_client_log_debug_error(client, __func__, __FILE__, __LINE__, message, ##__VA_ARGS__);



#endif