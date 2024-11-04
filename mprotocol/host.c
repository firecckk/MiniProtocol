#include "host.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int create_packet(Packet * packet, uint32_t id, uint8_t type, uint16_t field_count, Field **fields)
{
    packet->id = id;
    packet->type = type;
    packet->field_count = field_count;
    packet->fields = fields;
    return 0;
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
        perror("Failed to allocate memory for field\n");
        //fprintf(stderr, "Failed to allocate memory for field\n");
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
            //perror("Failed to read from file descriptor");
            return -1;
        }
        else if (result == 0)
        {
            fprintf(stderr, "Zero readings\n");
            return -2;
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
            //fprintf(stderr, "Read %d/%zu bytes: ", return_value, count);
            //for (size_t i = 0; i < count; i++) {
            //    fprintf(stderr, "%02X ", ((unsigned char*)buf)[i]);
            //}
            //fprintf(stderr, "\n");
            return return_value;
        } else if(_bytes_read > count)
        {
            // read more than expected
            perror("Read more bytes than expected");
            return -1;
        }
    } 
    // not enough bytes read, wait for next read
    return _bytes_read;
}

// initial state
enum _States _state = WAIT_SOP;
Packet _packet;
int _handled_field_count=0;
Field * _field;

// if complete a packet, return 0
// if failed to read, return -1
// if failed to handle the field, return -2
int parse_packet(int fd, FieldHandler handler)
{
    int read_len = 0;
    int total_read_len = 0;
    switch (_state)
    {
    case WAIT_SOP:
        read_len = _cached_read(fd, &_packet.sop, sizeof(TYPE_SOP));
        if ( read_len == sizeof(TYPE_SOP) && _packet.sop== SOP)
        {
            total_read_len += read_len;
            _state = WAIT_ID;
        } else {
            //fprintf(stderr, "Failed to read SOP, read: %02X\n", (&_packet)->sop);
            return(total_read_len == 0 ? read_len : total_read_len);
        }
        __attribute__((fallthrough));
    case WAIT_ID:
        read_len = _cached_read(fd, &((&_packet)->id), sizeof(TYPE_PACKET_ID));
        if ( read_len == sizeof(TYPE_PACKET_ID))
        {
            total_read_len += read_len;
            _state = WAIT_TYPE;
        } else {
            return(total_read_len == 0 ? read_len : total_read_len);
        }
        __attribute__((fallthrough));
    case WAIT_TYPE:
        read_len = _cached_read(fd, &((&_packet)->type), sizeof(TYPE_PACKET_TYPE));
        if ( read_len == sizeof(TYPE_PACKET_TYPE))
        {
            total_read_len += read_len;
            _state = WAIT_COUNT;
        } else {
            return(total_read_len == 0 ? read_len : total_read_len);
        }
        __attribute__((fallthrough));
    case WAIT_COUNT:
        read_len = _cached_read(fd, &((&_packet)->field_count), sizeof(TYPE_PACKET_FIELD_COUNT));
        if ( read_len == sizeof(TYPE_PACKET_FIELD_COUNT))
        {
            total_read_len += read_len;
            _state = WAIT_FIELD_TYPE;
        } else {
            return(total_read_len == 0 ? read_len : total_read_len);
        }
        __attribute__((fallthrough));
    case WAIT_FIELD_TYPE:
        _field = (Field *)malloc(sizeof(Field));
        read_len = _cached_read(fd, &(_field->type), sizeof(TYPE_FIELD_TYPE));
        if (read_len == sizeof(TYPE_FIELD_TYPE))
        {
            total_read_len += read_len;
            _state = WAIT_FIELD_DATA;
            //fprintf(stderr, "Field type: %d, with size: %u\n", _field->type, get_field_size(_field->type));
            _field->data = (uint8_t *)malloc(get_field_size(_field->type));
            if (_field->data == NULL)
            {
                perror("Failed to allocate memory for field data");
                fprintf(stderr, "Failed to allocate memory for field data\n");
                return -2;
            }
        } else {
            free(_field);
            return(total_read_len == 0 ? read_len : total_read_len);
        }
        __attribute__((fallthrough));
    case WAIT_FIELD_DATA:
        read_len = _cached_read(fd, _field->data, get_field_size(_field->type));
        if ( read_len == get_field_size(_field->type))
        {
            total_read_len += read_len;
            if(handler((&_packet)->id, _field) < 0)
            {
                // failed to handle the field
                free(_field->data);
                free(_field);
                fprintf(stderr, "Failed to handle the field\n");
                return -2;
            }
            free(_field->data);
            free(_field);
            _handled_field_count++;
            if(((&_packet)->field_count) == _handled_field_count)
            {
                // all fields are handled
                _state = WAIT_EOP;
            } else {
                _state = WAIT_FIELD_TYPE;
                return(total_read_len == 0 ? read_len : total_read_len);
            }
        } else {
            return(total_read_len == 0 ? read_len : total_read_len);
        }
        __attribute__((fallthrough));
    case WAIT_EOP:
        read_len = _cached_read(fd, &((&_packet)->eop), sizeof(TYPE_PACKET_EOP));
        if ( read_len == sizeof(TYPE_PACKET_EOP) && (&_packet)->eop == EOP)
        {
            total_read_len += read_len;
            _state = COMPLETE;
            return(total_read_len == 0 ? read_len : total_read_len);
        } else {
            //fprintf(stderr, "Failed to read EOP, read: %02X\n", (&_packet)->eop);
            return(total_read_len == 0 ? read_len : total_read_len);
        }
        __attribute__((fallthrough));
    case COMPLETE:
        // call parse_next() to reset the state
        return 0;
    }
    //return -1;
}

void parse_next()
{
    _state = WAIT_SOP;
    _handled_field_count = 0;
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
        printf("Field %d Type: %02X\n", i, packet->fields[i]->type);
        printf("Field %d Data: ", i);
        for (int j = 0; j < get_field_size(packet->fields[i]->type); j++)
        {
            printf("%02X", packet->fields[i]->data[j]);
        }
        printf("%c", EOP);
    }
}

void print_packet_to_file(Packet *packet, FILE *file)
{
    if (packet == NULL)
    {
        fprintf(stderr, "Invalid packet\n");
        return;
    }
    fprintf(file, "Packet ID: %d\n", packet->id);
    fprintf(file, "Packet Type: %d\n", packet->type);
    fprintf(file, "Field count: %d\n", packet->field_count);
    for (int i = 0; i < packet->field_count; i++)
    {
        fprintf(file, "Field %d Type: %02X\n", i, packet->fields[i]->type);
        fprintf(file, "Field %d Data: ", i);
        for (int j = 0; j < get_field_size(packet->fields[i]->type); j++)
        {
            fprintf(file, "%02X", packet->fields[i]->data[j]);
        }
        fprintf(file, "%c", EOP);
    }
}