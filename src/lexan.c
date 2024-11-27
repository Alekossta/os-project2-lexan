#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "../include/ConsoleReader.h"
#include "../include/Hashtable.h"
#include <sys/select.h>
#include <poll.h>
#include <fcntl.h>

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
    // read console arguments
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

    // Create pipes for splitter to builder communication
    int numberOfBuilders = consoleArguments.numOfBuilders;
    int splitterBuilderPipes[numberOfBuilders][2];
    for (int i = 0; i < numberOfBuilders; i++) 
    {
        pipe(splitterBuilderPipes[i]);
    }

    // listen for signals from splitters when they finish (they are killed).
    if (signal(SIGUSR1, handleUSR1) == SIG_ERR) {
        perror("Error setting up SIGUSR1 handler");
        exit(EXIT_FAILURE);
    }
    // dasdsdas

    int numberOfSplitters = consoleArguments.numOfSplitters;
    int numberOfLinesEachSplitter = numberOfLines / numberOfSplitters;
    int remainingLines = numberOfLines % numberOfSplitters;

    int splittersWorkload[numberOfSplitters];
    for(int i = 0; i < numberOfSplitters; i++)
    {
        splittersWorkload[i] = numberOfLinesEachSplitter;
    }

    int index = 0;
    while(remainingLines != 0)
    {
        splittersWorkload[index]++;
        index++;
        remainingLines--;

        if((index+1) == numberOfSplitters)
        {
            index = 0;
        }
    }

    int startingLine = 1;
    // spawn splitters
    for(int i = 0; i < numberOfSplitters; i++)
    {
        pid_t pid = fork();
        if(pid == 0)
        {
            for (int j = 0; j < numberOfBuilders; j++) {
                close(splitterBuilderPipes[j][0]); // we dont need to read in splitter
                dup2(splitterBuilderPipes[j][1], j + 3);
                close(splitterBuilderPipes[j][1]);
            }

            char numberOfLinesEachSplitterString[20];
            sprintf(numberOfLinesEachSplitterString, "%d", splittersWorkload[i]);

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
            startingLine += splittersWorkload[i];
        }
    }

    // Builder to root pipes and also fds for polling when reading from builders
    // in the root
    int builderRootPipes[numberOfBuilders][2];
    struct pollfd fds[numberOfBuilders];
    for (int i = 0; i < numberOfBuilders; i++) 
    {
        pipe(builderRootPipes[i]);
    }

    // listen for signal from builders when they dies/finish
    if (signal(SIGUSR2, handleUSR2) == SIG_ERR) {
        perror("Error setting up SIGUSR2 handler");
        exit(EXIT_FAILURE);
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
                close(splitterBuilderPipes[j][1]); // we dont need to write in builders
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
                close(builderRootPipes[j][0]); // we wont read in builders
                if(j != i)
                {
                    // we dont care about other builders
                    close(builderRootPipes[j][1]);
                }
                else
                {
                    dup2(builderRootPipes[j][1], 3);
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
        else
        {
            close(builderRootPipes[i][1]);
            fds[i].fd = builderRootPipes[i][0];
            fds[i].events = POLLIN;
        }
    }

    for (int i = 0; i < numberOfBuilders; i++) {
        close(splitterBuilderPipes[i][0]);
        close(splitterBuilderPipes[i][1]);
    }

    // wait for splitters to finish before sending data from builders to root
    for(int i = 0; i < numberOfSplitters; i++)
    {
        wait(NULL);
    }

    HashTable* rootHashTable = hashtableCreate(20000);
    int num_fds = numberOfBuilders;
    while(num_fds > 0)
    {
        int ret = poll(fds, numberOfBuilders, -1);
        if(ret > 0)
        {
            for(int i = 0; i < numberOfBuilders; i++)
            {
                if(fds[i].revents & POLLIN)
                {
                    char buffer[1024];
                    ssize_t bytes_read = read(fds[i].fd, buffer, sizeof(buffer));
                    if(bytes_read > 0)
                    {
                        // We assume the data format is "word frequency\n"
                        buffer[bytes_read] = '\0';
                        char *line = strtok(buffer, "\n");
                        while(line != NULL)
                        {
                            char word[256];
                            int frequency;
                            if(sscanf(line, "%s %d", word, &frequency) == 2)
                            {
                                hashtableInsert(rootHashTable, word, frequency);
                            }
                            line = strtok(NULL, "\n");
                        }
                    }
                    else if(bytes_read == 0)
                    {
                        close(fds[i].fd);
                        fds[i].fd = -1;
                        num_fds--;
                    }
                }
                else if(fds[i].revents & (POLLHUP | POLLERR))
                {
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    num_fds--;
                }
            }
        }
    }

    // Wait for builders to finish
    for(int i = 0; i < numberOfBuilders; i++)
    {
        wait(NULL);
    }

    // print the hashtable and free
    hashtablePrintAndWriteTopK(rootHashTable, consoleArguments.topPopular,
    consoleArguments.outputFileName);
    hashtableFree(rootHashTable);

    // print signals
    printf("USR1 signals: %d\n", USR1);
    printf("USR2 signals: %d\n", USR2);

    return 0;
}