#include "general/platformlib/platform/platform.h"
#include "general/platformlib/networking/networking.h"


int main(int argc, char* argv[]) {

	networking_init();

	void* server;
	uint32_t connect_result = networking_server_init(12345, &server);
	
	if (connect_result != NETWORKING_SUCCESS) return 0;

	void* client;

	while (1) {
		sleep_for_ms(10);
		if (networking_server_accept(server, &client) != NETWORKING_SUCCESS) continue;

		char message[] = "Hello CLient";

		uint32_t data_sent;
		networking_send_data(client, message, sizeof(message), &data_sent);

		char shutdown_buffer[32];
		for (int i = 0; i < 100; i++) {
			if (networking_receive_data(client, shutdown_buffer, sizeof(shutdown_buffer), shutdown_buffer) == NETWORKING_ERROR_SOCKET_CLOSED) {
				printf("server gracefully closed connection\n");
				break;
			}
			sleep_for_ms(1);
		}

		networking_signal_shutdown(client);
		networking_close_connection(client);
	}

	networking_exit();

	return 0;
}