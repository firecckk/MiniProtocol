#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#include "mprotocol.h"

int open_serial_port(const char *device) {
    int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("open_serial_port: Unable to open port");
        return -1;
    }

    struct termios options;
    tcgetattr(fd, &options);

    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);

    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    tcsetattr(fd, TCSANOW, &options);

    return fd;
}

Packet * generate_packet() {
    // create a packet
    u_int8_t field_data[FIELD_SIZE_HELLO] = {0x01, 0x02, 0x03};
    Field *field = create_field(FIELD_TYPE_HELLO, field_data);

    u_int8_t field_data2[FIELD_SIZE_GOODBYE] = {0x04};
    Field *field2 = create_field(FIELD_TYPE_GOODBYE, field_data2);

    Field fields[] = {*field, *field2};
    Packet *packet = create_packet(111, 0, 2, fields);

    print_packet(packet);

    return packet;
}

int main() {
    const char *device = "/dev/ttyS0"; // Change this to your serial port
    int fd = open_serial_port(device);
    if (fd == -1) {
        perror("Failed to open serial port");
        return 1;
    }

    Packet *packet = generate_packet();
    if (packet == NULL) {
        close(fd);
        return 1;
    }

    free_packet(packet);
    close(fd);
    return 0;
}