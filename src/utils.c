#include <time.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "../include/utils.h"
#include "../include/logger.h"

#define MAX_TIME 96

char* getTime()
{
    static char timeStr[MAX_TIME];
    char *format = "%d-%m-%Y %H:%M:%S";
    struct tm *currentTime;
    time_t timer;

    time(&timer);
    currentTime = localtime(&timer);

    strftime(timeStr, MAX_TIME, format, currentTime);
    return timeStr;
}

int isIP(const char *IP)
{
    char buf[sizeof(struct in6_addr)];

    if (inet_pton(AF_INET, IP, buf))
    {
        return IPv4;
    }
    else if (inet_pton(AF_INET6, IP, buf))
    {
        return IPv6;
    }
    else
    {
        return 0;
    }

}