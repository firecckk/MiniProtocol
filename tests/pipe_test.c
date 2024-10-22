#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int verbose = 0;

    // Check for the -v parameter
    if (argc > 1 && strcmp(argv[1], "-v") == 0) {
        verbose = 1;
    }

    char buffer[1024];
    ssize_t bytesRead;

    // Read from stdin and write to stdout
    while ((bytesRead = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
        if (verbose) {
            fprintf(stderr, "Read %zd bytes\n", bytesRead);
        }
        write(STDOUT_FILENO, buffer, bytesRead);
    }

    if (verbose) {
        fprintf(stderr, "Finished reading from stdin\n");
    }

    return 0;
}