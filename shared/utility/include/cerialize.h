#pragma once
#include <stdint.h>
#include <stddef.h>

struct cerialized_file_system_entry {
    struct cerialized_file_system_entry* next;
    uint8_t* name;
    uint8_t* data;
    size_t size;
};

struct cerialized_file_system {
    uint32_t table_length;
    struct cerialized_file_system_entry** entries;
};

void* cerialized_get_file(struct cerialized_file_system* fs, uint8_t* name, size_t* size);
