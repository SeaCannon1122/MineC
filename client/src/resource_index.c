#include "resource_index.h"
#include "resources.h"

const void* resource_index_query(uint8_t* path, size_t* size)
{
	return cerialized_get_file(cerialized_resources_file_system, path, size);
}