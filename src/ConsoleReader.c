#include "ConsoleReader.h"
#include "string.h"

ConsoleArguments readConsole(int argumentsCount, char* arguments[])
{
    ConsoleArguments consoleArguments;

    for(int i = 1; i < argumentsCount; i++)
    {
        if(strcmp(arguments[i], "-i") == 0 && i + 1 < argumentsCount)
        {
            i++;
            consoleArguments.inputFileName = arguments[i];
        }
        if(strcmp(arguments[i], "-l") == 0 && i + 1 < argumentsCount)
        {
            i++;
            consoleArguments.numOfSplitters = atoi(arguments[i]);
        }
        if(strcmp(arguments[i], "-m") == 0 && i + 1 < argumentsCount)
        {
            i++;
            consoleArguments.numOfBuilders = atoi(arguments[i]);
        }
        if(strcmp(arguments[i], "-t") == 0 && i + 1 < argumentsCount)
        {
            i++;
            consoleArguments.topPopular = atoi(arguments[i]);
        }
        if(strcmp(arguments[i], "-e") == 0 && i + 1 < argumentsCount)
        {
            i++;
            consoleArguments.exclusionListFileName = arguments[i];
        }
        if(strcmp(arguments[i], "-o") == 0 && i + 1 < argumentsCount)
        {
            i++;
            consoleArguments.outputFileName = arguments[i];
        }      
    }

    return consoleArguments;
}