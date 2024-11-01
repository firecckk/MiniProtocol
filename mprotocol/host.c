#include "host.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

Packet *create_packet(uint32_t id, uint8_t type, uint16_t field_count, Field **fields)
{
    Packet *packet = (Packet *)malloc(sizeof(Packet));
    if (packet == NULL)
    {
        // Handle memory allocation failure
        fprintf(stderr, "Failed to allocate memory for packet\n");
        // exit(EXIT_FAILURE);
    }
    packet->id = id;
    packet->type = type;
    packet->field_count = field_count;
    packet->fields = fields;
    return packet;
}

void free_packet(Packet *packet)
{
    if (packet != NULL)
    {
        for (int i = 0; i < packet->field_count; i++)
        {
            free_field(packet->fields[i]);
        }
        free(packet->fields);
        free(packet);
    }
}

Field *create_field(uint8_t type, uint8_t *data)
{
    Field *field = (Field *)malloc(sizeof(Field));
    if (field == NULL)
    {
        // Handle memory allocation failure
        fprintf(stderr, "Failed to allocate memory for field\n");
        // exit(EXIT_FAILURE);
    }
    field->type = type;
    field->data = data;
    return field;
}

void free_field(Field *field)
{
    if (field != NULL)
    {
        free(field);
    }
}

size_t _bytes_read = 0; // Number of bytes read so far
size_t _last_count = 0; 
void *_buf;
int _cached_read(int fd, void *buf, size_t count)
{
    // since non-blocking read, we need to cache the states.
    // buf and count should exactly the same as last call 
    // if it was not completed
    if(_bytes_read != 0 && _last_count != count)
    {
        perror("Cached read count does not match\n");
        return -1;
    }
    if(_bytes_read !=0 && _buf != buf)
    {
        perror("Cached read buffer does not match\n");
        return -1;
    }

    if (_bytes_read < count)
    {
        int result = read(fd, buf + _bytes_read, count - _bytes_read);
        if (result == -1)
        {
            fprintf(stderr, "Failed to read from file descriptor: %s\n", (char *) buf);
            perror("Failed to read from file descriptor");
            return -1;
        }
        else if (result == 0)
        {
            fprintf(stderr, "Zero readings\n");
            return -1;
        }
        _bytes_read += result;
        _last_count = count;
        _buf = buf;
        if (_bytes_read == count)
        {
            // complete last reading, reset the states
            int return_value = _bytes_read;
            _bytes_read = 0;
            _last_count = 0;
            return return_value;
        }
    } 
    // not enough bytes read, wait for next read
    return _bytes_read;
}

// initial state
enum _States _state = WAIT_SOP;
Packet _packet;
int _handled_field_count=0;
Field _field;

// if complete a packet, return 0
int parse_packet(int fd, FieldHandler handler)
{
    switch (_state)
    {
    case WAIT_SOP:
        if (_cached_read(fd, &((&_packet)->sop), sizeof(TYPE_SOP)) == sizeof(TYPE_SOP) && (&_packet)->sop == SOP)
        {
            _state = WAIT_ID;
        } else {
            fprintf(stderr, "Failed to read SOP, read: %c\n", (&_packet)->sop);
            return -1;
        }
    case WAIT_ID:
        if (_cached_read(fd, &((&_packet)->id), sizeof(TYPE_PACKET_ID)) == sizeof(TYPE_PACKET_ID))
        {
            _state = WAIT_TYPE;
        } else {
            return -1; // !!! TODO: Implement error handling. e.g. return -1 if failed to read
                        // e.g. return -2 if read not complete
        }
    case WAIT_TYPE:
        if (_cached_read(fd, &((&_packet)->type), sizeof(TYPE_PACKET_TYPE)) == sizeof(TYPE_PACKET_TYPE))
        {
            _state = WAIT_COUNT;
        } else {
            return -1;
        }
    case WAIT_COUNT:
        if (_cached_read(fd, &((&_packet)->field_count), sizeof(TYPE_PACKET_FIELD_COUNT)) == sizeof(TYPE_PACKET_FIELD_COUNT))
        {
            _state = WAIT_FIELD_TYPE;
        } else {
            return -1;
        }
    case WAIT_FIELD_TYPE:
        if (_cached_read(fd, &((&_field)->type), sizeof(TYPE_FIELD_TYPE)) == sizeof(TYPE_FIELD_TYPE))
        {
            _state = WAIT_FIELD_DATA;
        } else {
            return -1;
        }
    case WAIT_FIELD_DATA:
        if (_cached_read(fd, &((&_field)->data), get_field_size((&_field)->type)) == get_field_size((&_field)->type))
        {
            _state = WAIT_EOP;
            handler((&_packet)->id, &_field);
            _handled_field_count++;
            if(((&_packet)->field_count) == _handled_field_count)
            {
                // all fields are handled
                _state = WAIT_EOP;
            }
        } else {
            return -1;
        }
    case WAIT_EOP:
        if (_cached_read(fd, &((&_packet)->eop), sizeof(TYPE_PACKET_EOP)) == sizeof(TYPE_PACKET_EOP) && (&_packet)->eop == EOP)
        {
            _state = COMPLETE;
            return 1;
        } else {
            return -1;
        }
    case COMPLETE:
        // reset states
        _state = WAIT_SOP;
        _handled_field_count = 0;
        return 0;
    }

    return 1;
}

// verified
void write_packet(Packet *packet, int fd)
{
    // Write the packet header
    char sop = SOP;
    if (write(fd, &sop, 1) == -1)
    {
        perror("Failed to write packet header");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Write the packet ID
    if (write(fd, &packet->id, sizeof(packet->id)) == -1)
    {
        perror("Failed to write packet ID");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Write the packet type
    if (write(fd, &packet->type, sizeof(packet->type)) == -1)
    {
        perror("Failed to write packet type");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Write the field count
    if (write(fd, &packet->field_count, sizeof(packet->field_count)) == -1)
    {
        perror("Failed to write field count");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Write each field
    for (int i = 0; i < packet->field_count; i++)
    {
        // Write the field type
        if (write(fd, &packet->fields[i]->type, sizeof(uint8_t)) == -1)
        {
            perror("Failed to write field type");
            close(fd);
            exit(EXIT_FAILURE);
        }

        // Write the field data
        if (write(fd, packet->fields[i]->data, get_field_size(packet->fields[i]->type)) == -1)
        {
            perror("Failed to write field data");
            close(fd);
            exit(EXIT_FAILURE);
        }
    }

    // Write the end of packet character
    char eop = EOP;
    if (write(fd, &eop, 1) == -1)
    {
        perror("Failed to write EOP");
        close(fd);
        exit(EXIT_FAILURE);
    }
}

void print_packet(Packet *packet)
{
    if (packet == NULL)
    {
        fprintf(stderr, "Invalid packet\n");
        return;
    }
    printf("Packet ID: %d\n", packet->id);
    printf("Packet Type: %d\n", packet->type);
    printf("Field count: %d\n", packet->field_count);
    for (int i = 0; i < packet->field_count; i++)
    {
        printf("Field %d Type: %d\n", i, packet->fields[i]->type);
        printf("Field %d Data: ", i);
        for (int j = 0; j < get_field_size(packet->fields[i]->type); j++)
        {
            printf("%02X", packet->fields[i]->data[j]);
        }
        printf("%c", EOP);
    }
}