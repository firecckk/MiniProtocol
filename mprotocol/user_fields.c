#include "user_fields.h"

#include <stdio.h>
#include <stdint.h>

int get_field_size(uint8_t type) {
    switch (type) {
        case FIELD_TYPE_HELLO:
            return FIELD_SIZE_HELLO;
        case FIELD_TYPE_DATA_1:
            return FIELD_SIZE_DATA_1;
        case FIELD_TYPE_GOODBYE:
            return FIELD_SIZE_GOODBYE;
        default:
            return -1;
    }
}

// example of a field handler
int handle_field(int packet_id, Field *field) {
    switch (field->type) {
        case FIELD_TYPE_HELLO:
            printf("ID: %d, Hello: %02X %02X %02X\n", packet_id, field->data[0], field->data[1], field->data[2]);
            break;
        case FIELD_TYPE_DATA_1:
            printf("Data 1: ");
            for (int i = 0; i < FIELD_SIZE_DATA_1; i++) {
                printf("%02X ", field->data[i]);
            }
            printf("\n");
            break;
        case FIELD_TYPE_GOODBYE:
            printf("Goodbye: %02X\n", field->data[0]);
            break;
        default:
            return -1;
    }
    return 0;
}