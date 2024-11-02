#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mprotocol.h"

int handler(int packet_id, Field *field) {
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

void read_packet(int fd) {
    while(parse_packet(fd, handler)!=0);
}

int main() {
    
    read_packet(STDIN_FILENO);
    
    return EXIT_SUCCESS;
}