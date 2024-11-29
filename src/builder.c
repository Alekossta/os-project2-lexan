#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/times.h>
#include <unistd.h>
#include "Hashtable.h"

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {

    // Time measurement variables
    struct tms start_times, end_times;
    double start_clock, end_clock;
    double ticks_per_second = (double)sysconf(_SC_CLK_TCK);

    start_clock = (double)times(&start_times);

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    ssize_t bufferBytes = 0;
    HashTable* frequencyTable = hashtableCreate(20000);

    // Read words from stdin and build the hashtable
    while ((bytes_read = read(STDIN_FILENO, buffer + bufferBytes, BUFFER_SIZE - bufferBytes - 1)) > 0) {
        bufferBytes += bytes_read;

        ssize_t start = 0;
        for (ssize_t i = 0; i < bufferBytes; i++) {
            if (buffer[i] == '\0') {
                hashtableInsert(frequencyTable, buffer + start, 1);
                start = i + 1;
            }
        }

        if (start < bufferBytes) {
            memmove(buffer, buffer + start, bufferBytes - start);
            bufferBytes -= start;
        } else {
            bufferBytes = 0;
        }
    }

    // After getting all the words from the splitter, send data to the root
    for (int i = 0; i < frequencyTable->size; i++) {
        HashNode* currentNode = frequencyTable->buckets[i];
        while (currentNode) {
            if (currentNode->key) {
                char* word = currentNode->key;
                int frequency = currentNode->value;
                char message[256];
                snprintf(message, sizeof(message), "%s %d\n", word, frequency);
                write(3, message, strlen(message));
            }
            currentNode = currentNode->next;
        }
    }

    // Time measurement end
    end_clock = (double)times(&end_times);

    double timeCPU = (double) ((end_times.tms_utime - start_times.tms_utime) + (end_times.tms_stime - start_times.tms_stime));
    double cpu_time = timeCPU / ticks_per_second;
    double real_time = (end_clock - start_clock) / ticks_per_second;

    // Send time data to root
    char time_message[256];
    snprintf(time_message, sizeof(time_message), "__END_OF_DATA__\nCPU_TIME %lf\nREAL_TIME %lf\n", cpu_time, real_time);
    write(3, time_message, strlen(time_message));

    // Close the pipe connecting to the root
    close(3);

    // Notify the root process
    pid_t parentId = getppid();
    kill(parentId, SIGUSR2);

    return 0;
}
