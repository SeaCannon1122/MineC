#include "testing.h"

#include "general/platformlib/networking.h"
#include "general/platformlib/platform/platform.h"



int main(int argc, char* argv[]) {

	platform_init();
	networking_init();

	testing_main();

	networking_exit();
	platform_exit();

	return 0;
}