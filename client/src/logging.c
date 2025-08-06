#include <minec_client.h>


void minec_client_log_info(struct minec_client* client, uint8_t* message, ...)
{
	printf("LOG: INFO ");

	va_list args;
	va_start(args, message);
	vprintf(message, args);
	va_end(args);

	printf("\n--------------------------------------------------------------------------------------------------\n");
}

void minec_client_log_error(struct minec_client* client, uint8_t* message, ...)
{
	printf("LOG: ERROR ");

	va_list args;
	va_start(args, message);
	vprintf(message, args);
	va_end(args);

	printf("\n--------------------------------------------------------------------------------------------------\n");
}

#ifdef MINEC_CLIENT_DEBUG_LOG

void minec_client_log_debug(struct minec_client* client, uint8_t* message, ...)
{
	printf("DEBUGLOG: ");

	va_list args;
	va_start(args, message);
	vprintf(message, args);
	va_end(args);

	printf("\n--------------------------------------------------------------------------------------------------\n");
}

void _minec_client_log_debug_l(struct minec_client* client, uint8_t* function, uint8_t* file, uint32_t line, uint8_t* message, ...)
{
	printf("DEBUGLOG: In %s at %s:%d | ", function, file, line);

	va_list args;
	va_start(args, message);
	vprintf(message, args);
	va_end(args);

	printf("\n--------------------------------------------------------------------------------------------------\n");
}
#endif