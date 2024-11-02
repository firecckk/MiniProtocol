#ifndef PACKET_H
#define PACKET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


#define SOP 0x2A  // Start of Packet
#define EOP '\n'  // End of Packet

#define TYPE_SOP uint8_t
#define TYPE_PACKET_ID uint32_t
#define TYPE_PACKET_TYPE uint8_t
#define TYPE_PACKET_FIELD_COUNT uint16_t
#define TYPE_FIELD_TYPE uint8_t
#define TYPE_PACKET_EOP uint8_t

typedef struct {
    TYPE_FIELD_TYPE type;
    uint8_t *data; // data can be of any type
} Field;

typedef struct {
    TYPE_SOP sop;   // Start of packet
    TYPE_PACKET_ID id;    // Packet ID
    TYPE_PACKET_TYPE type;   // Packet type
    TYPE_PACKET_FIELD_COUNT field_count; // Number of fields
    Field **fields; // Packet body (multiple different type of fields)
    TYPE_PACKET_EOP eop;   // End of packet
} Packet;

#endif // PACKET_H