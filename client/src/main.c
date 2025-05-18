#include "minec_client.h"

int main(int argc, char* argv[])
{
	
	struct minec_client client;
	uint32_t game_return_status = minec_client_run(&client, "runtime_files/");

	return 0;
}