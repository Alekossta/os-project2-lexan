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
#include <string.h>

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

#define BUFFER_SIZE 4096

typedef struct {
    char buffer[BUFFER_SIZE];
    int buffer_len;
    int expecting_time_data;
} BuilderData;

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

    double builder_cpu_times[numberOfBuilders];
    double builder_real_times[numberOfBuilders];
    BuilderData buildersData[numberOfBuilders];
    for(int i = 0; i < numberOfBuilders; i++)
    {
        buildersData[i].buffer_len = 0;
        buildersData[i].expecting_time_data = 0;
        builder_cpu_times[i] = 0.0;
        builder_real_times[i] = 0.0;
    }

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
                    BuilderData *builder = &buildersData[i];

                    ssize_t bytes_read = read(fds[i].fd, builder->buffer + builder->buffer_len, BUFFER_SIZE - builder->buffer_len - 1);

                    if(bytes_read > 0)
                    {
                        builder->buffer_len += bytes_read;
                        builder->buffer[builder->buffer_len] = '\0';

                        char *line_start = builder->buffer;
                        char *newline_pos;

                        while((newline_pos = strchr(line_start, '\n')) != NULL)
                        {
                            *newline_pos = '\0'; // Null-terminate the line

                            char *line = line_start;

                            // Process the line
                            if(strcmp(line, "__END_OF_DATA__") == 0)
                            {
                                builder->expecting_time_data = 1;
                            }
                            else if(builder->expecting_time_data)
                            {
                                double value;
                                if(sscanf(line, "CPU_TIME %lf", &value) == 1)
                                {
                                    builder_cpu_times[i] = value;
                                }
                                else if(sscanf(line, "REAL_TIME %lf", &value) == 1)
                                {
                                    builder_real_times[i] = value;
                                }
                            }
                            else
                            {
                                // Process word frequency data
                                char word[256];
                                int frequency;
                                if(sscanf(line, "%s %d", word, &frequency) == 2)
                                {
                                    hashtableInsert(rootHashTable, word, frequency);
                                }
                            }

                            // Move to next line
                            line_start = newline_pos + 1;
                        }

                        // Move any remaining data to the beginning of the buffer in order to handle
                        // the case where we have read a partial a line (without \n)
                        int remaining = builder->buffer_len - (line_start - builder->buffer); // remaining number of bytes from line_start to the end of the buffer
                        memmove(builder->buffer, line_start, remaining); // move the remaining the data to the start of the buffer
                        builder->buffer_len = remaining; // prepare buffer for next read
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

    // Print builder time data
    for(int i = 0; i < numberOfBuilders; i++)
    {
        printf("Builder %d: CPU TIME %lf, REAL TIME %lf\n"
        , i+1, builder_cpu_times[i], builder_real_times[i]);
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
