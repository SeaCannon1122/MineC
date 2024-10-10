#pragma once 

#define MAX_USERNAME_LENGTH 63
#define MAX_PASSWORD_LENGTH 63

#define CONECTION_CLOSE_MESSAGE_LENGTH 255
#define MAX_CHAT_MESSAGE_LENGTH 255
#define MAX_SERVER_MESSAGE_LENGTH 511

struct global_position {
	int chunk_x;
	int chunk_y;
	int chunbk_z;

	float x;
	float y;
	float z;
};

struct direction {
	float roll;
	float pitch;
	float yaw;
};