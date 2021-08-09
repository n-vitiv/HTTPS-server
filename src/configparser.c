#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "../include/configparser.h"
#include "../include/utils.h"

void configParse(configParser *cParser)
{
    FILE *fp = fopen(cParser->filePath, "r");
    if (fp == NULL)
    {
        printf("configParseStart: %s.\n", strerror(errno));
        return;
    }

    char *separator, *buffer = NULL;
    size_t buflen = 0;
    cParser->keyCount = 0;

    //start reading file
    while (getline(&buffer, &buflen, fp) != -1)
    {
        if (*buffer != '\"')
        {
            printf("configParse: buffer != \"\n");
            continue;
        }

        char *token;
        token = strtok(buffer, "\"");
        if (token == NULL)
        {
            printf("configParse: token == NULL\n");
            continue;
        }
        strcpy(cParser->keys[cParser->keyCount].keyWord, token);
        
        token = strtok(NULL, "\"");
        if (*token != ':' || strlen(token) != 1)
        {
            printf("configParse: token == NULL\n");
            continue;
        }

        token = strtok(NULL, "\"");
        if (token == NULL)
        {
            printf("configParse: token == NULL\n");
            continue;
        }
        if (strstr(token, "\n"))
        {
            printf("configParse : bad token\n");
            continue;
        }
        strcpy(cParser->keys[cParser->keyCount].value, token);

        cParser->keyCount++;
    }

    free(buffer);
    if (fp != NULL)
    {
        fclose(fp);
    }
}

void setParams(http *httpStruct, configParser cParser)
{
    strcpy(httpStruct->ip, getValue("address", &cParser));
    if (httpStruct->ip == NULL || !isIP(httpStruct->ip))
    {
        strcpy(httpStruct->ip, "127.0.0.1");
    }

    httpStruct->port = atoi(getValue("port", &cParser));
    if (httpStruct->port <= 0 || httpStruct->port > 65535)
    {
        httpStruct->port = 80;
    }

    strcpy(httpStruct->rootFolder, getValue("root", &cParser));
    if (httpStruct->rootFolder == NULL)
    {
        strcpy(httpStruct->rootFolder, "pages");
    }

    strcpy(httpStruct->defaultDocument, getValue("default", &cParser));
    if (httpStruct->defaultDocument == NULL)
    {
        strcpy(httpStruct->defaultDocument, "index.html");
    }
}

char* getValue(char *key, configParser *cParser)
{
    for (int i = 0; i < cParser->keyCount; ++i)
    {
        if (!strcmp(cParser->keys[i].keyWord, key))
        {
            return cParser->keys[i].value;
        }
    }
    return 0;
}