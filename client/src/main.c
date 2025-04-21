#include "minec_client.h"

#include <window/window.h>
#include <networking/networking.h>

int main(int argc, char* argv[]) {
	
	window_init_system();
	networking_init();

	struct minec_client client;
	uint32_t game_return_status = minec_client_run(&client, "runtime_files/");

	if (game_return_status != 0) {
		printf("Press RETURN to exit ...");
		getchar();
	}

	networking_exit();
	window_deinit_system();

	return 0;
}