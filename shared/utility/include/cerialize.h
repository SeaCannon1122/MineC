#pragma once
#include <stdint.h>
#include <stddef.h>

struct cerialized_file_system_entry {
    struct cerialized_file_system_entry* next;
    const uint8_t* name;
    const uint8_t* data;
    size_t size;
};

struct cerialized_file_system {
    uint32_t table_length;
    struct cerialized_file_system_entry** entries;
};

const void* cerialized_get_file(const struct cerialized_file_system* fs, uint8_t* name, size_t* size);
