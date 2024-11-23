// builder.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024  // Adjust as needed

int main(int argc, char* argv[]) {
    char buffer[BUFFER_SIZE];  // Buffer to hold input data
    ssize_t bytes_read;
    ssize_t total_bytes = 0;

    while ((bytes_read = read(STDIN_FILENO, buffer + total_bytes, BUFFER_SIZE - total_bytes - 1)) > 0) {
        total_bytes += bytes_read;

        ssize_t start = 0;
        for (ssize_t i = 0; i < total_bytes; i++) {
            if (buffer[i] == '\0') {
                // Null terminator found; process the word
                buffer[i] = '\0';  // Ensure null termination

                // Process the word from buffer[start] to buffer[i]
                printf("Builder [%d] received: %s\n", getpid(), buffer + start);

                // Move 'start' to the next character after the null terminator
                start = i + 1;
            }
        }

        // Move any remaining data to the beginning of the buffer
        if (start < total_bytes) {
            memmove(buffer, buffer + start, total_bytes - start);
            total_bytes -= start;
        } else {
            // All data processed
            total_bytes = 0;
        }

        // If the buffer is full, but no null terminator was found, handle the error
        if (total_bytes == BUFFER_SIZE - 1) {
            fprintf(stderr, "Error: Word too long in builder [%d]\n", getpid());
            // Reset buffer
            total_bytes = 0;
        }
    }

    if (bytes_read == -1) {
        perror("Error reading from stdin");
        exit(EXIT_FAILURE);
    }

    // Optional: Perform any cleanup or final processing here

    // Optionally send a signal to the parent process to indicate completion
    // kill(getppid(), SIGUSR2);

    return 0;
}
