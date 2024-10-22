#ifndef FIELD_TYPES_H
#define FIELD_TYPES_H

#include "mprotocol.h"

// Field types
#define FIELD_TYPE_HELLO 0x01
#define FIELD_SIZE_HELLO 3

#define FIELD_TYPE_DATA_8 0x02
#define FIELD_SIZE_DATA_8 8

#define FIELD_TYPE_GOODBYE 0x10
#define FIELD_SIZE_GOODBYE 1

int get_field_size(uint8_t type);

int handle_field(int packet_id, Field *field);

#endif // FIELD_TYPES_H