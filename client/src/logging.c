#include <minec_client.h>


void minec_client_log(struct minec_client* client, uint8_t* prefixes, uint8_t* message, ...)
{
	printf("%s ", prefixes);

	va_list args;
	va_start(args, message);
	vprintf(message, args);
	va_end(args);

	printf("\n");
}

void minec_client_log_v(struct minec_client* client, uint8_t* prefixes, uint8_t* message, va_list args)
{
	printf("%s ", prefixes);
	vprintf(message, args);
	printf("\n");
}