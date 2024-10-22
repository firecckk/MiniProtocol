#include "host.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

Packet * create_packet(uint32_t id, uint8_t type, uint16_t field_count, Field **fields) {
    Packet *packet = (Packet *)malloc(sizeof(Packet));
    if (packet == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Failed to allocate memory for packet\n");
        //exit(EXIT_FAILURE);
    }
    packet->id = id;
    packet->type = type;
    packet->field_count = field_count;
    packet->fields = fields;
    return packet;
}

void free_packet(Packet *packet) {
    if (packet != NULL) {
        for (int i = 0; i < packet->field_count; i++) {
            free_field(packet->fields[i]);
        }
        free(packet->fields);
        free(packet);
    }
}

Field * create_field(uint8_t type, uint8_t *data) {
    Field * field = (Field *)malloc(sizeof(Field));
    if (field == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Failed to allocate memory for field\n");
        //exit(EXIT_FAILURE);
    }
    field->type = type;
    field->data = data;
    return field;
}

void free_field(Field *field) {
    if (field != NULL) {
        free(field);
    }
}

void parse_packet(int fd, FieldHandler handler) {
    uint32_t packet_id;
    uint8_t packet_type;
    uint16_t field_count;

    // Read packet ID
    if (read(fd, &packet_id, sizeof(packet_id)) != sizeof(packet_id)) {
        perror("Failed to read packet ID");
        close(fd);
        return;
    }

    // Read packet type
    if (read(fd, &packet_type, sizeof(packet_type)) != sizeof(packet_type)) {
        perror("Failed to read packet type");
        close(fd);
        return;
    }

    // Read field count
    if (read(fd, &field_count, sizeof(field_count)) != sizeof(field_count)) {
        perror("Failed to read field count");
        close(fd);
        return;
    }

    // Read each field
    for (size_t i = 0; i < field_count; i++) {
        Field * field = (Field *)malloc(sizeof(Field));

        if (read(fd, &field->type, sizeof(field->type)) != sizeof(field->type)) {
            perror("Failed to read field type");
            free(field);
            close(fd);
            return;
        }

        int field_size = get_field_size(field->type);
        if (field_size < 0) {
            perror("Invalid field type");
            free(field);
            close(fd);
            return;
        }

        uint8_t raw_data[field_size];
        if (read(fd, raw_data, field_size) != field_size) {
            perror("Failed to read field data");
            free(field);
            close(fd);
            return;
        }
        field->data = raw_data;

        if (handler != NULL) {
            handler(packet_id, field);
        } else {
            perror("No handler provided");
        }

        //free(field->data);
        free(field);
    }

    close(fd);
}

void write_packet(Packet *packet, int fd) {
    // Write the packet ID
    if (write(fd, &packet->id, sizeof(packet->id)) == -1) {
        perror("Failed to write packet ID");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Write the packet type
    if (write(fd, &packet->type, sizeof(packet->type)) == -1) {
        perror("Failed to write packet type");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Write the field count
    if (write(fd, &packet->field_count, sizeof(packet->field_count)) == -1) {
        perror("Failed to write field count");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Write each field
    for (int i = 0; i < packet->field_count; i++) {
        // Write the field type
        if (write(fd, &packet->fields[i]->type, sizeof(packet->fields[i]->type)) == -1) {
            perror("Failed to write field type");
            close(fd);
            exit(EXIT_FAILURE);
        }

        // Write the field data
        if (write(fd, packet->fields[i]->data, FIELD_SIZE_HELLO) == -1) {
            perror("Failed to write field data");
            close(fd);
            exit(EXIT_FAILURE);
        }
    }

    // Write the end of packet character
    char eop = EOP;
    if (write(fd, &eop, sizeof(EOP)) == -1) {
        perror("Failed to write EOP");
        close(fd);
        exit(EXIT_FAILURE);
    }
}

void print_packet(Packet *packet) {
    if (packet == NULL) {
        fprintf(stderr, "Invalid packet\n");
        return;
    }
    printf("Packet ID: %d\n", packet->id);
    printf("Packet Type: %d\n", packet->type);
    printf("Field count: %d\n", packet->field_count);
    for (int i = 0; i < packet->field_count; i++) {
        printf("Field %d Type: %d\n", i, packet->fields[i]->type);
        printf("Field %d Data: ", i);
        for (int j = 0; j < get_field_size(packet->fields[i]->type); j++) {
            printf("%02X", packet->fields[i]->data[j]);
        }
        printf("%c", EOP);
    }
}