#ifndef MINEC_CLIENT_LOGGING
#define MINEC_CLIENT_LOGGING

#include <stdint.h>
#include <stdarg.h>
#include <utils.h>

struct minec_client;

#ifndef MINEC_CLIENT_INCLUDE_ONLY_STRUCTURE
void minec_client_log_info(struct minec_client* client, uint8_t* message, ...);
void minec_client_log_error(struct minec_client* client, uint8_t* message, ...);
#endif

#ifdef MINEC_CLIENT_DEBUG_LOG

#ifndef MINEC_CLIENT_INCLUDE_ONLY_STRUCTURE
void minec_client_log_debug(struct minec_client* client, uint8_t* message, ...);
void _minec_client_log_debug_l(struct minec_client* client, uint8_t* function, uint8_t* file, uint32_t line, uint8_t* message, ...);
#endif

#define minec_client_log_debug_l(client, message, ...) {_minec_client_log_debug_l(client, __func__, __FILE__, __LINE__, message, ##__VA_ARGS__); DEBUGBREAK();}
#define minec_client_log_out_of_memory(client, prefixes, call, ...) {minec_client_log_error(client, "%s Out of memory", prefixes); minec_client_log_debug_l(client, "'%s' failed -> Out of memory", call, ##__VA_ARGS__);}

#else

#ifndef MINEC_CLIENT_INCLUDE_ONLY_STRUCTURE
#define minec_client_log_debug(client, message, ...)
#endif
#define minec_client_log_debug_l(client, message, ...)
#define minec_client_log_out_of_memory(client, call) minec_client_log_error(client, "Out of memory")

#endif

#endif