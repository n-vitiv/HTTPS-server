#include <stdio.h>
#include <errno.h>
#include <openssl/ssl.h>

#include "../include/network.h"
#include "../include/logger.h"

int netClose(int conn)
{
    int closeId = close(conn);
    if (closeId == -1)
    {
        printLog(LOG_ERROR, "netClose: %s.\n", strerror(errno));
    }
    return closeId;
}

int netSend(SSL *ssl, char* buffer, int size)
{
    return SSL_write(ssl, buffer, size);
}

int netRecv(SSL *ssl, char* buffer, int size)
{
    return SSL_read(ssl, buffer, size);
}

int netAccept(int listener)
{
    int acceptId = accept(listener, 0, 0);
    if (acceptId < 0)
    {
        printLog(LOG_ERROR, "netAccept: %s.\n", strerror(errno));
        return acceptId;
    }

    //setting options for closing socket
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1;
    if (setsockopt(acceptId, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0)
    {
        printLog(LOG_ERROR, "netAccept: %s.\n", strerror(errno));
    }

    return acceptId;
}

int netListen(char* address, int port)
{
    //creating socket for listening
    int listening = socket(AF_INET6, SOCK_STREAM, 0);
    if (listening < 0)
    {
        printLog(LOG_ERROR, "netListen: %s.\n", strerror(errno));
        return -1;
    }

    int no = 0;
    if (setsockopt(listening, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&no, sizeof(no)))
    {
        printLog(LOG_ERROR, "netListen: %s.\n", strerror(errno));
        return -1;
    }

    //filling the structure sockaddr_in
    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(struct sockaddr_in6));
    addr.sin6_port = htons(port);
    addr.sin6_family = AF_INET6;

    if (inet_pton(AF_INET6, address, &addr.sin6_addr) < 0)
    {
        printLog(LOG_ERROR, "netListen: %s\n", strerror(errno));
        return -1;
    }

    //binding sockaddr_in with socket
    if (bind(listening, (struct sockaddr*)&addr, sizeof(addr)) != 0)
    {
        printLog(LOG_ERROR, "netListen: %s.\n", strerror(errno));
        return -1;
    }

    //setting options for closing socket
    if (setsockopt(listening, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    {
        printLog(LOG_ERROR, "netListen: %s\n", strerror(errno));
        return -1;
    }

    //setting socket for listening
    if (listen(listening, SOMAXCONN) != 0)
    {
        printLog(LOG_ERROR, "netListen: %s.\n", strerror(errno));
        return -1;
    }

    return listening;
}
