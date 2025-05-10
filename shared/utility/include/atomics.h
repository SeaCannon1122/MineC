#pragma once

#ifndef ATOMICS_H
#define ATOMICS_H

#include "mutex.h"
#include <stdint.h>

#define atomic_(type) struct { uint8_t mutex_and_data[sizeof(mutex_t) + sizeof(type)];}
#define atomic_store_(type, p_atomic, p_data) {mutex_lock((mutex_t*)(p_atomic)); memcpy((void*)&((uint8_t*)(p_atomic))[sizeof(mutex_t)], (void*)(p_data), (size_t)sizeof(type)); mutex_unlock((mutex_t*)(p_atomic));}
#define atomic_load_(type, p_atomic, p_data) {mutex_lock((mutex_t*)(p_atomic)); memcpy((void*)(p_data), (void*)&((uint8_t*)(p_atomic))[sizeof(mutex_t)], (size_t)sizeof(type)); mutex_unlock((mutex_t*)(p_atomic));}

#define atomic_init(p_atomic) {mutex_create((mutex_t*)(p_atomic));}
#define atomic_deinit(p_atomic) {mutex_destroy((mutex_t*)(p_atomic));}

#endif
