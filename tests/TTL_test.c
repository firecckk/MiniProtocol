#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "mprotocol.h"
#include "utill.c"

double get_current_time_microseconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)(tv.tv_sec) * 1e6 + (double)(tv.tv_usec);
}

double calculate_delay(double start, double end) {
    double micro_seconds = end - start;
    return micro_seconds;
}

void handle_TTL_field(int packet_id, Field *field) {
    double start_time;
    memcpy(&start_time, field->data, FIELD_SIZE_TTL_START_TIME);
    double end_time = get_current_time_microseconds();
    double delay = calculate_delay(start_time, end_time);
    printf("Packet ID: %d, Delay: %.3f ms\n", packet_id, delay / 1000.0);
}

int handler(int packet_id, Field *field) {
    switch (field->type) {
        case FIELD_TYPE_HELLO:
            printf("ID: %d, Hello: %02X %02X %02X\n", packet_id, field->data[0], field->data[1], field->data[2]);
            break;
        case FIELD_TYPE_DATA_256:
            printf("Data 256: ");
            for (int i = 0; i < FIELD_SIZE_DATA_256; i++) {
                if (field->data[i] != 'A') {
                    printf("incorrect");
                    return -1;
                }
            }
            printf("correct");
            printf("\n");
            break;
        case FIELD_TYPE_TTL_START_TIME: 
            handle_TTL_field(packet_id, field);
            break;
        case FIELD_TYPE_GOODBYE:
            printf("Goodbye: %02X\n", field->data[0]);
            break;
        default:
            return -1;
    }
    return 0;
}

int send_TTL_packet(int fd, int packet_id) {
    Packet packet;

    Field field;
    field.type = FIELD_TYPE_DATA_256;
    field.data = (uint8_t *) malloc(FIELD_SIZE_DATA_256);
    memset(field.data, 'A', FIELD_SIZE_DATA_256);

    Field field2;
    field2.type = FIELD_TYPE_TTL_START_TIME;
    field2.data = (uint8_t *) malloc(FIELD_SIZE_TTL_START_TIME);
    
    double start_time = get_current_time_microseconds();
    memcpy(field2.data, &start_time, FIELD_SIZE_TTL_START_TIME);

    Field *fields[] = {&field, &field2};
    create_packet(&packet, packet_id, 0, 2, fields);

    FILE *file = fopen("TTL_packet.text", "wb");
    print_packet_to_file(&packet, file);
    fclose(file);

    write_packet(&packet, fd);
    free(field.data);
    free(field2.data);

    return 0;
}

int main() {
    const char *device = "/dev/cu.usbmodem14201"; // Change this to your serial port
    int fd = open_serial_port(device);
    if (fd == -1) {
        perror("Failed to open serial port");
        return 1;
    }
    fprintf(stderr, "Serial port opened\n");

    send_TTL_packet(fd, 1);

    int result;
    do{
        result = parse_packet(fd, handler);
        if(result==0) {
            fprintf(stderr, "packet handled complete\n");
        }else if(result==-1) {
            //fprintf(stderr, "Resources is busy\n");
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