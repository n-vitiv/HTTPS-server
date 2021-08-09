#include <stdio.h>
#include <string.h>
#include "include/http.h"
#include "include/logger.h"
#include "include/configparser.h"
#include "include/ssl.h"

int main()
{
    configParser cParser;
    strcpy(cParser.filePath, "config.txt");
    configParse(&cParser);

    loggerInit(cParser);

    http *httpStruct = httpInit(cParser);
    setParams(httpStruct, cParser);

    httpListen(httpStruct);

    httpDeinit(httpStruct);
    loggerDeinit();
    return 0;
}
