#include <minec_client.h>


void minec_client_log(struct minec_client* client, uint8_t* message, ...)
{
	va_list args;
	va_start(args, message);
	vprintf(message, args);
	va_end(args);

	printf("\n");
}

void minec_client_log_error(struct minec_client* client, uint8_t* message, ...)
{
	va_list args;
	va_start(args, message);
	vprintf(message, args);
	va_end(args);

	printf("\n");
}

void _minec_client_log_debug_error(struct minec_client* client, uint8_t* function, uint8_t* file, uint32_t line, uint8_t* message, ...)
{
	printf("[DEBUG] [ERROR] in %s at %s:%d | ", function, file, line);

	va_list args;
	va_start(args, message);
	vprintf(message, args);
	va_end(args);

	printf("\n");
}