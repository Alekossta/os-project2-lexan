#pragma once

// structure that stores arguments read from the console
// pointers dont need to be freed because they are passed to us by the OS
// and get freed at the end of the program
typedef struct ConsoleArguments
{
    char* inputFileName;
    int numOfSplitters;
    int numOfBuilders;
    int topPopular;
    char* exclusionListFileName;
    char* outputFileName;
} ConsoleArguments;

ConsoleArguments readConsole(int argumentsCount, char* arguments[]);