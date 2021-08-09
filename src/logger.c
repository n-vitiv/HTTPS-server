#include <stdio.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include "../include/logger.h"
#include "../include/utils.h"

static FILE *fLog;
static FILE *fw3cLog;

static int isEnabled = 1;

pthread_mutex_t mutex;

void loggerInit(configParser cParser)
{
    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        printf("Mutex creation failed.\n");
    }
    struct stat st = {0};

    //creating directory for logs
    if (stat("logs", &st) == -1)
    {
        umask(0);
        mkdir("logs", S_IRWXU | S_IRWXG | S_IRWXO);
    }

    umask(0);
    //opening file for simple logs
    fLog = fopen("logs/logs.txt", "w");
    if (fLog == NULL)
    {
        printf("Can`t open file for simple logs.\n");
    }

    //opening file for w3c logs
    fw3cLog = fopen("logs/w3clogs.txt", "w");
    if (fw3cLog == NULL)
    {
        printf("Can`t open file for w3c logs.\n");
    }
    else
    {
        fprintf(fw3cLog, "#Version: 1.0\n#Date: %s\nFields: time cs-method cs-uri\n", getTime());
        fflush(fw3cLog);
    }

    for (int i = 0; i < cParser.keyCount; ++i)
    {
        if (!strcmp(cParser.keys[i].keyWord, "logs"))
        {
            isEnabled = atoi(cParser.keys[i].value);
            break;
        }
    }

    if (isEnabled >= 4)
    {
        isEnabled = 1;
    }
}

void loggerDeinit()
{  
    if (fLog != NULL)
    {
        fclose(fLog);
    }
    if (fw3cLog != NULL)
    {
        fclose(fw3cLog);
    }
    if (pthread_mutex_destroy(&mutex) != 0)
    {
        printf("Mutex destoying failed.\n");
    }
}

void printLog(logLevel level, char *msg, ...)
{
    if (isEnabled == 0)
    {
        return;
    }
    va_list args;
    va_start(args, msg);

    if (pthread_mutex_lock(&mutex) != 0)
    {
        printf("Can`t lock mutex.\n");
    }

    if (fLog == NULL)
    {
        printf("Can`t print logs.\n");
    }
    else
    {
        switch (level)
        {
            case LOG_DEBUG:
                if (isEnabled >= 2)
                {
                    break;
                }
                fprintf(fLog,"DEBUG: %s ", getTime());
                vfprintf(fLog, msg, args);
                fflush(fLog);
                break;
            
            case LOG_ERROR:
                fprintf(fLog,"ERROR: %s ", getTime());
                vfprintf(fLog, msg, args);
                fflush(fLog);
                break;
            
            case LOG_WARNING:
                if (isEnabled >= 3)
                {
                    break;
                }
                fprintf(fLog,"WARNING: %s ", getTime());
                vfprintf(fLog, msg, args);
                fflush(fLog);
                break;

            default:

                break;
        }
    }

    if (level == LOG_W3C)
    {
        if (fw3cLog == NULL)
        {
            printf("Can`t print W3C logs.\n");
        }
        else
        {
            fprintf(fw3cLog, "%s ", getTime());
            vfprintf(fw3cLog, msg, args);
            fflush(fw3cLog);
        }
    }

    va_end(args);
    if (pthread_mutex_unlock(&mutex) != 0)
    {
        printf("Can`t unlock mutex.\n");
    }
}
