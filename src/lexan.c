#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "../include/ConsoleReader.h"
#include "../include/Utility.h"

unsigned USR1 = 0;
void handleUSR1(int sig)
{
    USR1++;
}

unsigned USR2 = 0;
void handleUSR2(int sig)
{
    USR2++;
}

int main(int argumentsCount, char* arguments[])
{
    ConsoleArguments consoleArguments = readConsole(argumentsCount, arguments);

    // input file. count number of lines
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

    // Create pipes
    int numberOfBuilders = consoleArguments.numOfBuilders;
    int splitterBuilderPipes[numberOfBuilders][2];
    for (int i = 0; i < numberOfBuilders; i++) 
    {
        pipe(splitterBuilderPipes[i]);
    }

    // spawn splitters
    int numberOfSplitters = consoleArguments.numOfSplitters;
    int numberOfLinesEachSplitter = numberOfLines / numberOfSplitters;
    int startingLine = 1;
    for(int i = 0; i < numberOfSplitters; i++)
    {
        pid_t pid = fork();
        if(pid == 0)
        {
            for (int j = 0; j < numberOfBuilders; j++) {
                close(splitterBuilderPipes[j][0]);
                dup2(splitterBuilderPipes[j][1], j + 3);
                close(splitterBuilderPipes[j][1]);
            }

            char numberOfLinesEachSplitterString[20];
            sprintf(numberOfLinesEachSplitterString, "%d", numberOfLinesEachSplitter);

            char startingLineString[20];
            sprintf(startingLineString, "%d", startingLine);

            char numberOfBuildersString[20];
            sprintf(numberOfBuildersString, "%d", numberOfBuilders);

            char* arguments[] = {"./bin/splitter", consoleArguments.inputFileName
            ,numberOfLinesEachSplitterString, startingLineString, consoleArguments.exclusionListFileName,  numberOfBuildersString
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

    if (signal(SIGUSR1, handleUSR1) == SIG_ERR) {
        perror("Error setting up SIGUSR1 handler");
        exit(EXIT_FAILURE);
    }

    // Builder to root pipes
    int builderRootPipes[numberOfBuilders][2];
    for (int i = 0; i < numberOfBuilders; i++) 
    {
        pipe(builderRootPipes[i]);
    }


    // spawn builders
    for(int i = 0; i < numberOfBuilders; i++)
    {
        pid_t pid = fork();
        if(pid == 0)
        {
            // We pass all write ends of pipe to our splitters
            for (int j = 0; j < numberOfBuilders; j++) {

                // handle splitter builder pipes
                close(splitterBuilderPipes[j][1]);
                if(j != i)
                {
                    // we dont care about other builders
                    close(splitterBuilderPipes[j][0]);
                }
                else
                {
                    dup2(splitterBuilderPipes[j][0], STDIN_FILENO);
                    close(splitterBuilderPipes[j][0]); // Close original read end
                }

                // handle builder root pipes
                close(builderRootPipes[j][0]);
                if(j != i)
                {
                    // we dont care about other builders
                    close(builderRootPipes[j][1]);
                }
                else
                {
                    dup2(builderRootPipes[j][1], j+3);
                    close(builderRootPipes[j][1]); // Close original read end
                }
            }


            char numberOfBuildersString[20];
            sprintf(numberOfBuildersString, "%d", numberOfBuilders);
            
            char* arguments[] = {"./bin/builder",NULL};
            execv(arguments[0], arguments);
            perror("did not create builder\n");
            return -1;
        }
    }

    if (signal(SIGUSR2, handleUSR2) == SIG_ERR) {
        perror("Error setting up SIGUSR2 handler");
        exit(EXIT_FAILURE);
    }

    // After creating all child processes
    for (int i = 0; i < numberOfBuilders; i++) {
        close(splitterBuilderPipes[i][0]); // Close read end
        close(splitterBuilderPipes[i][1]); // Close write end
    }


    // wait for splitters to finish
    for(int i = 0; i < numberOfSplitters; i++)
    {
        wait(NULL);
    }

    // read data coming from builders

    // wait for builders to finish
    for(int i = 0; i < numberOfBuilders; i++)
    {
        wait(NULL);
    }

    printf("USR1 signals: %d\n", USR1);
    printf("USR2 signals: %d\n", USR2);

    return 0;
}