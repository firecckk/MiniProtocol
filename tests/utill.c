#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

int open_serial_port(const char *device) {
    int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
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