#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mprotocol.h"

void read_packet(int fd, Packet * packet) {
    parse_packet(fd, handle_field);
}

int main() {

    Packet packet;
    read_packet(STDIN_FILENO, &packet);
    
    return EXIT_SUCCESS;
}