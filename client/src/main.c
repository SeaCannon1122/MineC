#include "minec_client.h"

#include "platformlib/platform/platform.h"
#include "platformlib/networking/networking.h"

int main(int argc, char* argv[]) {

	platform_init();
	networking_init();

	show_console_window();

	struct minec_client client;
	uint32_t game_return_status = minec_client_run(&client, "runtime_files/");

	if (game_return_status != 0) {
		printf("Press RETURN to exit ...");
		getchar();
	}

	networking_exit();
	platform_exit();

	return 0;
}