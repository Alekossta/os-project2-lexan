#include "stdio.h"
#include "../include/ConsoleReader.h"

int main(int argumentsCount, char* arguments[])
{
    ConsoleArguments consoleArguments = readConsole(argumentsCount, arguments);
    printf("%s\n", consoleArguments.inputFileName);
    return 0;
}