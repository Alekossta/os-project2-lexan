#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "../include/ConsoleReader.h"
#include "../include/Utility.h"

int main(int argumentsCount, char* arguments[])
{
    ConsoleArguments consoleArguments = readConsole(argumentsCount, arguments);

    // input file
    int numberOfLines = 0;
    FILE* inputFile = fopen(consoleArguments.inputFileName, "r");
    if(inputFile)
    {
        char character;
        while((character = fgetc(inputFile)) != EOF)
        {
            if(character == '\n')
            {
                numberOfLines++;
            }
        }        
    }
    else
    {
        printf("Error opening input file\n");
        return -1;
    }
    
    fclose(inputFile);

    unsigned USR1 = 0;
    // spawn splitters
    int numberOfSplitters = consoleArguments.numOfSplitters;
    int numberOfLinesEachSplitter = numberOfLines / numberOfSplitters;
    int startingLine = 1;
    for(int i = 0; i < numberOfSplitters; i++)
    {
        pid_t pid = fork();
        if(pid == 0)
        {
            char numberOfLinesEachSplitterString[20]; // Large enough for most integers
            sprintf(numberOfLinesEachSplitterString, "%d", numberOfLinesEachSplitter);

            char startingLineString[20];
            sprintf(startingLineString, "%d", startingLine);

            char* arguments[] = {"./bin/splitter", consoleArguments.inputFileName
            ,numberOfLinesEachSplitterString, startingLineString, consoleArguments.exclusionListFileName
            ,NULL};

            execv(arguments[0], arguments);
            perror("did not create splitter\n");
            return -1;
        }
        else // we are on parent
        {
            startingLine += numberOfLinesEachSplitter;
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

    return 0;
}