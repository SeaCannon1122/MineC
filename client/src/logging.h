#ifndef MINEC_CLIENT_LOGGING
#define MINEC_CLIENT_LOGGING

#include <stdint.h>
#include <stdarg.h>
#include <utils.h>

struct minec_client;

void minec_client_log_info(struct minec_client* client, uint8_t* message, ...);
void minec_client_log_error(struct minec_client* client, uint8_t* message, ...);

#ifdef MINEC_CLIENT_DEBUG_LOG

void _minec_client_log_debug_error(struct minec_client* client, uint8_t* function, uint8_t* file, uint32_t line, uint8_t* message, ...);
#define minec_client_log_debug_error(client, message, ...) {_minec_client_log_debug_error(client, __func__, __FILE__, __LINE__, message, ##__VA_ARGS__); DEBUGBREAK();}
#define minec_client_log_out_of_memory(client, call) {minec_client_log_error(client, "Out of memory"); minec_client_log_debug_error(client, "'%s' failed -> Out of memory", call);}

#else

#define minec_client_log_debug_error(client, message, ...)
#define minec_client_log_out_of_memory(client, call) minec_client_log_error(client, "Out of memory")

#endif

#endif