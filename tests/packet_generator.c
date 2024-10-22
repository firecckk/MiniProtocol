#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mprotocol.h"

void generate_packet() {
    // create a packet
    uint8_t field_data[FIELD_SIZE_HELLO] = {0x01, 0x02, 0x03};
    Field field;
    field.type = FIELD_TYPE_HELLO;
    field.data = field_data;

    uint8_t field_data2[FIELD_SIZE_GOODBYE] = {0x04};
    Field field2;
    field2.type = FIELD_TYPE_GOODBYE;
    field2.data = field_data2;

    Field *fields[] = {&field, &field2};
    Packet *packet = create_packet(1, 2, 2, fields);

    //print_packet(packet);
    write_packet(packet, STDOUT_FILENO);
     
    fflush(stdout);
}

//int main(int argc, char *argv[]) {
int main() {
    // if (argc != 2) {
    //     fprintf(stderr, "Usage: %s <input>\n", argv[0]);
    //     return EXIT_FAILURE;
    // }
    // const char *input = argv[1];

    generate_packet();

    return EXIT_SUCCESS;
}