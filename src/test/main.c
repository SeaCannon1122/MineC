#include "testing.h"

#define GLEW_STATIC
#include <GL/glew.h>

#include "general/platformlib/parallel_computing.h"
#include "general/platformlib/networking.h"
#include "general/platformlib/platform.h"
#include "general/platformlib/opengl_rendering.h"



int main(int argc, char* argv[]) {

	platform_init();
	networking_init();
	parallel_computing_init();
	opengl_init();

	glewInit();

	testing_main();

	opengl_exit();
	parallel_computing_exit();
	networking_exit();
	platform_exit();

	return 0;
}