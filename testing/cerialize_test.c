#include <cerialize.h>
#include <assets.h>

#include <stdio.h>

int main()
{

	size_t size;
	uint8_t* data = cerialized_get_file(cerialized_assets_file_system, "icon.png", &size);

}