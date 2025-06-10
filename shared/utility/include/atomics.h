#pragma once

#ifndef ATOMICS_H
#define ATOMICS_H

#include "mutex.h"
#include <stdint.h>

#define _ATOMICS_H_MUTEX_OFFSET (((sizeof(mutex_t) + 7) / 8) * 8)

#define atomic_(type) struct { uint8_t mutex_and_data[_ATOMICS_H_MUTEX_OFFSET + sizeof(type)];}
#define atomic_store_(type, p_atomic, p_data) {mutex_lock((mutex_t*)(p_atomic)); memcpy((void*)&((uint8_t*)(p_atomic))[_ATOMICS_H_MUTEX_OFFSET], (void*)(p_data), (size_t)sizeof(type)); mutex_unlock((mutex_t*)(p_atomic));}
#define atomic_load_(type, p_atomic, p_data) {mutex_lock((mutex_t*)(p_atomic)); memcpy((void*)(p_data), (void*)&((uint8_t*)(p_atomic))[_ATOMICS_H_MUTEX_OFFSET], (size_t)sizeof(type)); mutex_unlock((mutex_t*)(p_atomic));}

#define atomic_init(p_atomic) {mutex_create((mutex_t*)(p_atomic));}
#define atomic_deinit(p_atomic) {mutex_destroy((mutex_t*)(p_atomic));}

#endif
