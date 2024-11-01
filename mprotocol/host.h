#ifndef HOST_H
#define HOST_H

#include "mprotocol.h"

Packet * create_packet(uint32_t id, uint8_t type, uint16_t field_count, Field **fields);

void free_packet(Packet *packet);

Field * create_field(uint8_t type, uint8_t *data);

void free_field(Field *field);

// call back function to handle different field types
typedef int (*FieldHandler)(int packet_id ,Field *field);
int parse_packet(int fd, FieldHandler handler);

void write_packet(Packet *packet, int fd);

void print_packet(Packet *packet);

enum _States {
    WAIT_SOP,
    WAIT_ID,
    WAIT_TYPE,
    WAIT_COUNT,
    WAIT_FIELD_TYPE,
    WAIT_FIELD_DATA,
    WAIT_EOP,
    COMPLETE
};

int field_count;

#endif // HOST_H