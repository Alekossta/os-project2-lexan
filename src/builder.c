#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "Hashtable.h"

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
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

        if (bufferBytes == BUFFER_SIZE - 1) {
            fprintf(stderr, "Error: Word too long in builder [%d]\n", getpid());
            bufferBytes = 0;
        }
    }

    if (bytes_read == -1) {
        perror("Error reading from stdin");
        exit(EXIT_FAILURE);
    }

    // after getting all the words from the splitter we can now send our data
    // to the root.
    // we will pass the words in the format = word frequency\n
    for(int i = 0; i < frequencyTable->size; i++)
    {
        HashNode* currentNode = frequencyTable->buckets[i];
        while(currentNode)
        {
            if(currentNode->key)
            {
                char* word = currentNode->key;
                int frequency = currentNode->value;
                char message[256];
                snprintf(message, sizeof(message), "%s %d\n", word, frequency);
                write(3, message, strlen(message));
            }
            currentNode = currentNode->next;
        }
    }

    // close the pipe connecting to the root
    close(3);

    // Notify the root process
    pid_t parentId = getppid();
    kill(parentId, SIGUSR2);

    return 0;
}
