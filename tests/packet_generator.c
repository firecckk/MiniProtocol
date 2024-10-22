#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mprotocol.h"

Packet * generate_packet() {
    // create a packet
    u_int8_t field_data[FIELD_SIZE_HELLO] = {0x01, 0x02, 0x03};
    Field *field = create_field(FIELD_TYPE_HELLO, field_data);

    u_int8_t field_data2[FIELD_SIZE_GOODBYE] = {0x04};
    Field *field2 = create_field(FIELD_TYPE_GOODBYE, field_data2);

    Field ** fields = {&field, &field2};
    Packet *packet = create_packet(111, 0, 2, fields);

    print_packet(packet);

    return packet;
}

int main(int argc, char *argv[]) {
    // if (argc != 2) {
    //     fprintf(stderr, "Usage: %s <input>\n", argv[0]);
    //     return EXIT_FAILURE;
    // }
    // const char *input = argv[1];

    Packet *packet = generate_packet();
    

    int fd = STDOUT_FILENO;

    write_packet(packet, fd);

    return EXIT_SUCCESS;
}