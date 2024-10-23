#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

int open_serial_port(const char *device) {
    int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    fprintf(stderr, "open_serial_port: Opening serial port\n");
    if (fd == -1) {
        perror("open_serial_port: Unable to open port");
        return -1;
    }

    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    tcsetattr(fd, TCSANOW, &options);

    return fd;
}

ssize_t read_from_serial_port(int fd, void *buffer, size_t size) {
    ssize_t bytes_read = read(fd, buffer, size);
    //if (bytes_read == -1) {
    //    perror("read_from_serial_port: Unable to read from port");
    //}
    return bytes_read;
}

int main() {
    const char *device = "/dev/tty.usbmodem14201"; // Change this to your serial port device
    int fd = open_serial_port(device);
    if (fd == -1) {
        return EXIT_FAILURE;
    }

    const char *message = "A"; // Character to send
    ssize_t bytes_written = write(fd, message, strlen(message));
    if (bytes_written == -1) {
        perror("main: Unable to write to port");
        close(fd);
        return EXIT_FAILURE;
    }

    char buffer[256];
    while (1) {
    write(fd, message, strlen(message));
    ssize_t bytes_read = read_from_serial_port(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("Read %zd bytes: %s\n", bytes_read, buffer);
    } else if (bytes_read == -1) {
        continue;
    }
    usleep(100000); // Sleep for 100ms to avoid busy-waiting
}

    close(fd);
    return EXIT_SUCCESS;
}
