#include "stdio.h"
#include "Hashtable.h"
#include <stdlib.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 1024
#define MAX_WORD_LENGTH 256

int is_word_char(char c) {
    return isalpha(c);  // Words consist of alphabetic characters
}

// arguments will have the form: ./splitter inputFileName numberOfLines startingLine exlusionListFileName
int main(int argumentsCount, char* arguments[])
{
    char* inputFileName=  arguments[1];
    int numberOfLines = atoi(arguments[2]);
    int startingLine = atoi(arguments[3]);
    char* exclusionListFileName = arguments[4];

    // exclusion file
    FILE* exclusionFile = fopen(exclusionListFileName, "r");
    HashTable* exclusionHashTable = hashtableCreate(4500);
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
                if (is_word_char(line[i])) {
                    word[word_index++] = tolower(line[i]);
                    if (word_index >= MAX_WORD_LENGTH - 1) {
                        word[word_index] = '\0';
                        if(hashtableSearch(exclusionHashTable, word) == -1)
                        {
                            //printf("%s\n", word);
                        }
                        else
                        {
                            printf("rejected: %s\n", word);
                        }

                        word_index = 0;
                    }
                } else if (word_index > 0) {
                    word[word_index] = '\0'; 
                    if(hashtableSearch(exclusionHashTable, word) == -1)
                    {
                        //printf("%s\n", word);
                    }
                    else
                    {
                        printf("rejected: %s\n", word);
                    }
                    word_index = 0;
                }
            }

            if (word_index > 0) {
                word[word_index] = '\0';
                if(hashtableSearch(exclusionHashTable, word) == -1)
                {
                    //printf("%s\n", word);
                }
                else
                {
                    printf("rejected: %s\n", word);
                }
            }
        }
    }
    else
    {
        printf("Error opening input file in splitter\n");
        return -1;
    }
    fclose(inputFile);
    return 0;
}