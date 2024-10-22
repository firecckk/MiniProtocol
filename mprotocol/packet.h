#ifndef PACKET_H
#define PACKET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @struct Field
 * @brief Represents a field in a packet.
 * 
 * @var Field::type
 * Type of the field.
 * 
 * @var Field::data
 * Pointer to the data of the field.
 * 
 * @var Field::length
 * Length of the data.
 */
typedef struct {
    uint8_t type;
    uint8_t *data;
} Field;

/**
 * @struct Packet
 * @brief Represents a packet in the communication protocol.
 * 
 * @var Packet::id
 * ID of the packet.
 * 
 * @var Packet::fields
 * Pointer to an array of fields in the packet.
 * 
 * @var Packet::field_count
 * Number of fields in the packet.
 */
typedef struct {
    uint32_t id;    // Packet ID
    uint8_t type;   // Packet type
    uint16_t field_count; // Number of fields
    Field **fields; // Packet body (multiple different type of fields)
} Packet;

#endif // PACKET_H