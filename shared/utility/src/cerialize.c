#include "cerialize.h"
#include <string.h>
#include <stdio.h>

uint32_t cerialized_hash(const uint8_t* str, uint32_t table_len) {
    uint32_t hash = 5381;
    while (*str) hash = ((hash << 5) + hash) + *str++;
    return hash % table_len;
}

const void* cerialized_get_file(const struct cerialized_file_system* fs, uint8_t* name, size_t* size) {
    uint32_t index = cerialized_hash(name, fs->table_length);
    struct cerialized_file_system_entry* entry = fs->entries[index];

    while (entry) {
        if (strcmp((const char*)entry->name, (const char*)name) == 0) {
            if (size) *size = entry->size;
            return entry->data;
        }
        entry = entry->next;
    }
    return NULL;
}
