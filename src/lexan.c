#include "stdio.h"
#include "../include/ConsoleReader.h"

int main(int argumentsCount, char* arguments[])
{
    ConsoleArguments consoleArguments = readConsole(argumentsCount, arguments);

    // input file
    FILE* inputFile = fopen(consoleArguments.inputFileName, "r");
    if(inputFile)
    {

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

    // do stuff


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