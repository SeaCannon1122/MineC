#include <minec_client.h>

void minec_client_log(struct minec_client* client, uint8_t* message, ...)
{
	va_list args;
	va_start(args, message);
	vprintf((const char* const)message, args);
	va_end(args);

	printf("\n");
}

void minec_client_log_v(struct minec_client* client, uint8_t* message, va_list args)
{
	vprintf((const char* const)message, args);
	printf("\n");
}