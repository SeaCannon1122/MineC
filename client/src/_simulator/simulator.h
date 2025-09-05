#pragma once

#ifndef SIMULATOR_H
#define SIMULATOR_H

struct stored_entity {

	float x;
	float y;
	float z;

	float pitch;
	float roll;
	float yaw;

	uint16_t type;

	uint16_t entity_data_index;
};

struct simulator_state {

	void* thread_handle;

	uint32_t thread_active_flag;
	uint32_t simulator_active_bool;

	uint8_t request_exit_flag;



};

struct minec_client;

uint32_t simulator_start(struct minec_client* game);

uint32_t simulator_stop(struct minec_client* game);

#endif // !SIMULATOR_H