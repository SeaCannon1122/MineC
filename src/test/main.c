#include <stdio.h>

#include <malloc.h>

#include "general/platformlib/platform/platform.h"
#include "general/platformlib/networking/networking.h"


int main(int argc, char* argv[]) {
	
	networking_init();

	void* server;
	uint32_t connect_result = networking_client_connect("127.0.0.1", 12345, &server);

	uint32_t connected = 0;

	for (uint32_t i = 0; i < 100; i++) {

		uint32_t r = networking_client_connection_status(server);

		if (r == NETWORKING_SUCCESS) {
			printf("conected to server on atempt %d\n", i);
			connected = 1;
			break;
		}
		else if (r == NETWORKING_ERROR_CONNECTING) printf("wait for server to accept %d\n", i);
		else if (r == NETWORKING_ERROR_COULD_NOT_CONNECT) {
			printf("couldn't connect to server\n"); break;
		}
		else {
			printf("Error retriving connection status\n");
			break;
		}

		sleep_for_ms(10);
	}

	if (connected == 0) return 0;



	char buffer[2048];
	uint32_t data_received;


	while(networking_receive_data(server, buffer, 2048, &data_received) == NETWORKING_ERROR_NOT_READY) sleep_for_ms(10);

	networking_signal_shutdown(server);
	
	char shutdown_buffer[32];
	for (int i = 0; i < 100; i++) {
		if (networking_receive_data(server,shutdown_buffer, sizeof(shutdown_buffer), shutdown_buffer) == NETWORKING_ERROR_SOCKET_CLOSED) {
			printf("server gracefully closed connection\n");
			break;
		}
		sleep_for_ms(1);
	}

	networking_close_connection(server);

	printf(buffer);

	sleep_for_ms(2000);

	networking_exit();

	return 0;
}