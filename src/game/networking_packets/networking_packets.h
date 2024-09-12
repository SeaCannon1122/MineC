#pragma once

enum networking_packet_type_client {
	NETWORKING_PACKET_MESSAGE_SMALL,
	NETWORKING_PACKET_MESSAGE_LARGE,
	NETWORKING_PACKET_CONECTION_STATE,
	NETWORKING_PACKET
};

enum connection_state_types {
	REQUEST_CONNECTION,
	DENY_CONNECTION,

};

struct networking_packet_message_small {
	char message[256];
};

struct networking_packet_message_large {
	char message[4069];
};