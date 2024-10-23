#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mprotocol.h"

void read_packet(int fd) {
    parse_packet(fd, handle_field);
}

int main() {
    
    read_packet(STDIN_FILENO);
    
    return EXIT_SUCCESS;
}