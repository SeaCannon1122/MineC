#include "testing.h"

#include "general/platformlib/platform/platform.h"

#include "client/rendering/gui/pixel_char.h"


int main(int argc, char* argv[]) {

	platform_init();

    show_console_window();



	platform_exit();

	return 0;
}