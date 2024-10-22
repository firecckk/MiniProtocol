#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include "mprotocol.h"

int main() {
    // Create a packet
    Packet *packet = (Packet *)malloc(sizeof(Packet));
    packet->id = 0;
    packet->field_count = 1;
    packet->fields = (Field **)malloc(sizeof(Field *) * packet->field_count);
    packet->fields[0] = (Field *)malloc(sizeof(Field));
    packet->fields[0]->type = FIELD_TYPE_HELLO;
    packet->fields[0]->data = (uint8_t *)malloc(FIELD_SIZE_HELLO);
    packet->fields[0]->data[0] = 0x01;
    packet->fields[0]->data[1] = 0x02;
    packet->fields[0]->data[2] = 0x03;

    // create a packet
    u_int8_t field_data[FIELD_SIZE_HELLO] = {0x01, 0x02, 0x03};
    Field *field = create_field(FIELD_TYPE_HELLO, field_data);

    Packet *packet2 = create_packet(111, 0, 1, &field);

    print_packet(packet2);
    
    // Open a file descriptor to write the packet
    int fd = open("packet_output.bin", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    
    write_packet(packet2, fd);

    // Close the file descriptor
    close(fd);

    // Free the packet
    for (int i = 0; i < packet->field_count; i++) {
        free(packet->fields[i]->data);
        free(packet->fields[i]);
    }
    free(packet->fields);
    free(packet);

    // read the file
    fd = open("packet_output.bin", O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    parse_packet(fd, handle_field);

    return 0;
}
