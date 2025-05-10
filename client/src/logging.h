#ifndef MINEC_CLIENT_LOGGING
#define MINEC_CLIENT_LOGGING

#include <stdint.h>
#include <stdarg.h>

void minec_client_log(struct minec_client* client, uint8_t* message, ...);
void minec_client_log_v(struct minec_client* client, uint8_t* message, va_list args);

#endif