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
    fcntl(fd, F_SETFL, 0); // Blocking mode
    if (fd == -1) {
        perror("open_serial_port: Unable to open port");
        return -1;
    }

    struct termios options;
    tcgetattr(fd, &options);

    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);

    // Enable the receiver and set local mode
    options.c_cflag |= (CLOCAL | CREAD);
    // Disable parity bit
    options.c_cflag &= ~PARENB;
    // Use 2 stop bit
    options.c_cflag &= ~CSTOPB;
    // Clear current character size mask
    options.c_cflag &= ~CSIZE;
    // Set character size to 8 bits
    options.c_cflag |= CS8;

    tcsetattr(fd, TCSANOW, &options);

    return fd;
}

void generate_packet(int fd) {
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
    Packet packet;
    create_packet(&packet, 1, 2, 2, fields);

    //print_packet(packet);
    write_packet(&packet, fd);
}

int main() {
    const char *device = "/dev/cu.usbmodem14201"; // Change this to your serial port
    int fd = open_serial_port(device);
    if (fd == -1) {
        perror("Failed to open serial port");
        return 1;
    }

    fprintf(stderr, "Serial port opened\n");

    char flag = 'A';
    if(write(fd, &flag, 1) == -1) {
        perror("Failed to write flag");
        close(fd);
        return 1;
    }

    fprintf(stderr, "Clearing the buffer\n");

    char c;
    do {
        // Clear buffer
        usleep(100000);
        read(fd, &c, 1);
        fprintf(stderr, "%s", &c);
    }
    while(c != flag);

    generate_packet(fd);
    parse_packet(fd, handle_field);

    close(fd);
    return 0;
}