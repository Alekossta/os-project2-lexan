#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "../include/ConsoleReader.h"

int main(int argumentsCount, char* arguments[])
{
    ConsoleArguments consoleArguments = readConsole(argumentsCount, arguments);

    // input file
    FILE* inputFile = fopen(consoleArguments.inputFileName, "r");
    if(inputFile)
    {
        int numberOfLines = 0;
        char character;
        while((character = fgetc(inputFile)) != EOF)
        {
            if(character == '\n')
            {
                numberOfLines++;
            }
        }
        printf("Number of lines is %d\n", numberOfLines);
    }
    else
    {
        printf("Error opening input file\n");
        return -1;
    }
    fclose(inputFile);

    // exclusion file
    FILE* exclusionFile = fopen(consoleArguments.exclusionListFileName, "r");
    if(exclusionFile)
    {
        
    }
    else
    {
        printf("Error opening exclusion file\n");
        return -1;
    }
    fclose(exclusionFile);

    unsigned USR1 = 0;
    // spawn splitters
    int numberOfSplitters = consoleArguments.numOfSplitters;
    for(int i = 0; i < numberOfSplitters; i++)
    {
        pid_t pid = fork();
        if(pid == 0)
        {
            char* arguments[] = {"./bin/splitter",NULL};
            execv(arguments[0], arguments);
            perror("did not create splitter\n");
            return -1;
        }
    }

    // wait for splitters to finish
    for(int i = 0; i < numberOfSplitters; i++)
    {
        wait(NULL);
    }

    unsigned USR2 = 0;
    // spawn builders
    int numberOfBuilders = consoleArguments.numOfBuilders;
    for(int i = 0; i < numberOfBuilders; i++)
    {
        pid_t pid = fork();
        if(pid == 0)
        {
            char* arguments[] = {"./bin/builder",NULL};
            execv(arguments[0], arguments);
            perror("did not create builder\n");
            return -1;
        }
    }

    // wait for builders to finish
    for(int i = 0; i < numberOfBuilders; i++)
    {
        wait(NULL);
    }

    printf("USR1 signals: %d\n", USR1);
    printf("USR2 signals: %d\n", USR2);

    // output file
    FILE* outputFile = fopen(consoleArguments.outputFileName, "w");
    if(outputFile)
    {

    }
    else
    {
        printf("Error opening output file\n");
        return -1;
    }
    fclose(outputFile);

    return 0;
}