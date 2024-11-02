#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#include "mprotocol.h"

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

    //print_packet(&packet);
    write_packet(&packet, fd);
}

int my_handler(int packet_id, Field *field) {
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

int open_serial_port(const char *device) {
    int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    //fcntl(fd, F_SETFL, 0); // Blocking mode
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

int main() {
    const char *device = "/dev/cu.usbmodem14201"; // Change this to your serial port
    int fd = open_serial_port(device);
    if (fd == -1) {
        perror("Failed to open serial port");
        return 1;
    }
    fprintf(stderr, "Serial port opened\n");

    generate_packet(fd);
    //usleep(1000000);
    parse_next();
    int result;
    do{
        result = parse_packet(fd, my_handler);
        if(result==0) {
            fprintf(stderr, "packet handled complete\n");
        }else if(result==-1) {
            fprintf(stderr, "Resources is busy\n");
        } else if(result == -2) {
            fprintf(stderr, "Severe error happened!!!!\n");
        } else {
            fprintf(stderr, "read %d bytes\n", result);
        }
    }
    while(result!=0);

    close(fd);
    return 0;
}