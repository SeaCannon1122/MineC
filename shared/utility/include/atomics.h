#pragma once

#ifndef ATOMICS_H
#define ATOMICS_H

#include <mutex.h>
#include <stdint.h>
#include <stdbool.h>

#define ATOMICS_DECLARATION_(type, type_name, function_prefix)\
typedef struct atomic_##type_name {mutex_t mutex; type value;} atomic_##type_name;\
function_prefix void atomic_##type_name##_init(atomic_##type_name* p_atomic, type initial_value);\
function_prefix void atomic_##type_name##_deinit(atomic_##type_name* p_atomic);\
function_prefix void atomic_##type_name##_store(atomic_##type_name* p_atomic, type value);\
function_prefix type atomic_##type_name##_load(atomic_##type_name* p_atomic);\

#define ATOMICS_IMPLEMENTATION_(type, type_name, function_prefix)\
function_prefix void atomic_##type_name##_init(atomic_##type_name* p_atomic, type initial_value)\
{\
	mutex_create(&p_atomic->mutex);\
	p_atomic-> value = initial_value;\
}\
function_prefix void atomic_##type_name##_deinit(atomic_##type_name* p_atomic)\
{\
	mutex_destroy(&p_atomic->mutex);\
}\
function_prefix void atomic_##type_name##_store(atomic_##type_name* p_atomic, type value)\
{\
	mutex_lock(&p_atomic->mutex);\
	p_atomic->value = value;\
	mutex_unlock(&p_atomic->mutex);\
}\
function_prefix type atomic_##type_name##_load(atomic_##type_name* p_atomic)\
{\
	type value;\
	mutex_lock(&p_atomic->mutex);\
	value = p_atomic->value;\
	mutex_unlock(&p_atomic->mutex);\
	return value;\
}\

ATOMICS_DECLARATION_(int8_t, int8_t, )
ATOMICS_DECLARATION_(uint8_t, uint8_t, )
ATOMICS_DECLARATION_(int16_t, int16_t, )
ATOMICS_DECLARATION_(uint16_t, uint16_t, )
ATOMICS_DECLARATION_(int32_t, int32_t, )
ATOMICS_DECLARATION_(uint32_t, uint32_t, )
ATOMICS_DECLARATION_(int64_t, int64_t, )
ATOMICS_DECLARATION_(uint64_t, uint64_t, )
ATOMICS_DECLARATION_(float, float, )
ATOMICS_DECLARATION_(double, double, )
ATOMICS_DECLARATION_(bool, bool, )

#endif
