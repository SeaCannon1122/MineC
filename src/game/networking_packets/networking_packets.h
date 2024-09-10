#pragma once

enum networking_packet_type {
	NETWORKING_PACKET_MESSAGE_SMALL,
	NETWORKING_PACKET_MESSAGE_LARGE,
};

struct networking_packet_message_small {
	char message[256];
};

struct networking_packet_message_large {
	char message[4069];
};