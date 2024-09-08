#pragma once

#include "keyvalue.h"

struct key_value_map* new_resource_manager(char* src_keyvalue);
void destroy_resource_manager(struct key_value_map* manager);