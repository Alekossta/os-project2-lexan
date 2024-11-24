#include "stdio.h"
#include "Hashtable.h"
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE_LENGTH 1024
#define MAX_WORD_LENGTH 256

int hash_function(const char *word, int num_builders) {
    unsigned long hash = 5381;
    int c;

    while ((c = *word++)) {
        hash = ((hash << 5) + hash) + c; 
    }

    return hash % num_builders;
}

// arguments will have the form: ./splitter inputFileName numberOfLines startingLine exlusionListFileName numberOfBuilders
int main(int argumentsCount, char* arguments[])
{
    char* inputFileName=  arguments[1];
    int numberOfLines = atoi(arguments[2]);
    int startingLine = atoi(arguments[3]);
    char* exclusionListFileName = arguments[4];
    int numberOfBuilders = atoi(arguments[5]);

    // MAKE THE EXCLUSIONS WORDS HASHTABLE
    HashTable* exclusionHashTable = hashtableCreate(4500);
    FILE* exclusionFile = fopen(exclusionListFileName, "r");
    if(exclusionFile)
    {
        char line[MAX_LINE_LENGTH];
        while(fgets(line, MAX_LINE_LENGTH, exclusionFile))
        {
            line[strcspn(line, "\n")] = '\0';
            hashtableInsert(exclusionHashTable, line, 0);
        }
    }
    else
    {
        printf("Error opening exclusion file\n");
        return -1;
    }
    fclose(exclusionFile);

    FILE* inputFile = fopen(inputFileName, "r");
    if(inputFile)
    {
        char line[MAX_LINE_LENGTH];
        int current_line = 0;

        while (fgets(line, MAX_LINE_LENGTH, inputFile)) {
            current_line++;

            if (current_line < startingLine)
                continue;

            if (current_line > startingLine + numberOfLines - 1)
                break;

            line[strcspn(line, "\n")] = '\0';

            char word[MAX_WORD_LENGTH];
            int word_index = 0;

            for (int i = 0; line[i] != '\0'; i++) {
                if (isalpha(line[i])) {
                    word[word_index++] = tolower(line[i]);
                    if (word_index >= MAX_WORD_LENGTH - 1) {
                        word[word_index] = '\0';
                        if(hashtableSearch(exclusionHashTable, word) == -1)
                        {
                            int index = hash_function(word, numberOfBuilders);
                            write(index+3,word, strlen(word) + 1);
                        }

                        word_index = 0;
                    }
                } else if (word_index > 0) {
                    word[word_index] = '\0'; 
                    if(hashtableSearch(exclusionHashTable, word) == -1)
                    {
                        int index = hash_function(word, numberOfBuilders);
                        write(index+3,word, strlen(word) + 1);

                    }
                    word_index = 0;
                }
            }

            if (word_index > 0) {
                word[word_index] = '\0';
                if(hashtableSearch(exclusionHashTable, word) == -1)
                {
                    int index = hash_function(word, numberOfBuilders);
                    write(index+3,word, strlen(word) + 1);
                }
            }
        }
        
        // close the read end to all builders
        for(int i = 0; i < numberOfBuilders; i++)
        {
            close(i+3);
        }
    }
    else
    {
        printf("Error opening input file in splitter\n");
        return -1;
    }
    fclose(inputFile);
    hashtableFree(exclusionHashTable);

    pid_t parentId = getppid();
    kill(parentId, SIGUSR1);
    
    return 0;
}