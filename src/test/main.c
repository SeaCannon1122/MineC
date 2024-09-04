#include "general/utils.h"
#include <stdio.h>
#include "general/keyvalue.h"


int main() {
    
    struct key_value_map map = key_value_map_from_file("../../../resources/client/assets/resourcelayout.keyvalue");

    for (int i = 0; i < map.mappings_count; i++) {
        printf("key: %s    value: ", map.mappings[i].key);
        if (map.mappings[i].value_type == VALUE_STRING) printf("%s", map.mappings[i].value.s);
        if (map.mappings[i].value_type == VALUE_INT) printf("%d", map.mappings[i].value.i);
        if (map.mappings[i].value_type == VALUE_FLOAT) printf("%f", map.mappings[i].value.f);
        printf("\n");
    }

    return 0;
}
