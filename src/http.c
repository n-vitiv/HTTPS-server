#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>

#include "../include/http.h"
#include "../include/network.h"
#include "../include/logger.h"

#define BUFSIZE 8192
#define THREADS 126356
#define CHUNK_SIZE 1024
#define CONTENT_SIZE 16

char* codeToStr(httpCode code);
void* threadHTTP(void *ptr);
void* threadHTTPS(void *ptr);

struct httpsArg
{
    http* httpStruct;
    int conn;
    SSL *ssl;
};

http* httpInit()
{
    //creating http
    http *httpStruct = (http*)malloc(sizeof(http));
    if (httpStruct == NULL)
    {
        printLog(LOG_ERROR, "httpInit: %s\n", strerror(errno));
        return NULL;
    }

    return httpStruct;
}

void httpDeinit(http* httpStruct)
{
    free(httpStruct);
}

void parseRequest(httpRequest *request, char *buffer, int size)
{
    request->isKeepAlive = 0;

    char *token = strtok(buffer, " ");
    if (token == NULL || strlen(token) > METHOD_SIZE)
    {
        printLog(LOG_ERROR, "parseRequest: Bad method size.\n");
        return;
    }
    strcpy(request->method, token);
    token = strtok(NULL, " ");
    if (token == NULL || strlen(token) > FILE_PATH_SIZE)
    {
        printLog(LOG_ERROR, "parseRequest: Bad file path size.\n");
        return ;
    }
    strcpy(request->filePath, token);

    token = strtok(NULL, " \r\n");
    if (token == NULL || strlen(token) > PROTOCOL_VERSION_SIZE)
    {
        printLog(LOG_ERROR, "parseRequest: Bad protocol version size.\n");
        return;
    }
    strcpy(request->protocolVersion, token);

    printLog(LOG_W3C, "%s %s \n" , request->method, request->filePath);

    while (1)
    {
        token = strtok(NULL, " ");
        if (strstr(token, "\r\n\r\n"))
        {
            break;
        }

        if (strstr(token, "Connection:"))
        {
            token = strtok(NULL, " \r\n");
            if (!strcmp(token, "keep-alive"))
            {
                request->isKeepAlive = 1;
            }
            break;
        }

        token = strtok(NULL, " \r\n");
    }
}

void createResponse(httpRequest *request, httpResponse *response, http *httpStruct)
{
    char temp[FILE_PATH_SIZE] = {0};
    strcpy(temp, request->filePath);
    strcpy(request->filePath, httpStruct->rootFolder);
    strcat(request->filePath, temp);

    if (!strcmp(temp, "/"))
    {
        printLog(LOG_DEBUG, "createResponse: file path : /.\n");
        strcat(request->filePath, httpStruct->defaultDocument);
    }

    if (strcmp(request->method, "GET"))
    {
        printLog(LOG_DEBUG, "createResponse: bad request.\n");
        response->code = HTTP_CODE_BAD_REQUEST;
        strcat(response->filePath, "pages/400.html");
    }
    else if (access(request->filePath, R_OK) != 0)
    {
        printLog(LOG_DEBUG, "createResponse: %s.\n", strerror(errno));
        response->code = HTTP_CODE_NOT_FOUND;
        strcat(response->filePath, "pages/404.html");
        request->isKeepAlive = 0;
    }
    else
    {
        char *filePath = canonicalize_file_name(request->filePath);
        char tempFilePath[BUFSIZE];
        getcwd(tempFilePath, BUFSIZE);
        strcat(tempFilePath, "/");
        strcat(tempFilePath, httpStruct->rootFolder);
        char *tempCanonFilePath = canonicalize_file_name(tempFilePath);

        if (strstr(filePath, tempCanonFilePath) != NULL)
        {
            response->code = HTTP_CODE_OK;
            strcat(response->filePath, request->filePath);
        }
        else
        {
            response->code = HTTP_CODE_FORBIDDEN;
            strcat(response->filePath, "pages/403.html");
        }
    }

    //calculating file size
    FILE *fp = fopen(response->filePath, "r"); 
    if (fp == NULL)
    {
        printLog(LOG_ERROR, "createResponse: %s\n", strerror(errno));
        return;
    }
    fseek(fp, 0L, SEEK_END);
    response->fileSize = ftell(fp);
    fclose(fp);
    
    response->isChunked = (response->fileSize < BUFSIZE) ? 0 : 1;

    strcat(response->protocolVersion, request->protocolVersion);
}

void sendResponse(httpResponse *response, int conn, SSL *ssl)
{
    //creating http header
    char buffer[BUFSIZE] = "";
    strcat(buffer, response->protocolVersion);
    strcat(buffer, codeToStr(response->code));
    strcat(buffer, "\r\nContent-Type: text/html; charset=UTF-8\r\n");
    
    if (response->isChunked)
    {
        strcat(buffer, "Transfer-Encoding: chunked\r\n\r\n");
        netSend(ssl, buffer, strlen(buffer));
    }
    else
    {
        strcat(buffer, "Content-Length: ");
        char size[CONTENT_SIZE];
        sprintf(size, "%d", response->fileSize);
        strcat(buffer, size);
        strcat(buffer, "\r\n\r\n");
    }

    int fp = open(response->filePath, O_RDONLY);
    if (fp == -1)
    {
        printLog(LOG_ERROR, "sendResponse: %s.\n", strerror(errno));
        return;
    }

    char str[BUFSIZE];
    int buflen;
    if (response->isChunked)
    {
        printLog(LOG_DEBUG, "Sending chunks.\n");
        sendChunked(fp, conn, response, ssl);
        close(fp);
    }
    else
    {
        printLog(LOG_DEBUG, "Sending full message.\n");
        sendNotChunked(fp, conn, buffer, response, ssl);
        close(fp);
    }
}

int httpListen(http* httpStruct)
{
    initOpenSSL();
    SSL_CTX *ctx = createContext();
    configureContext(ctx);
    
    pthread_t thread[THREADS];
    int threadCount = 0;

    int listener = netListen(httpStruct->ip, httpStruct->port);
    //check if socket set to listening
    if (listener < 0)
    {
        SSL_CTX_free(ctx);
        cleanupOpenSSL();
        return -1;
    }
    
    while (1)
    {
        int conn = netAccept(listener);
        //check if socket accepted
        if (conn < 0)
        {
            continue;
        }

        SSL *ssl = setSSL(ctx, conn);

        struct httpsArg args;
        args.httpStruct = httpStruct;
        args.conn = conn;
        args.ssl = ssl;
    
        if ((SSL_accept(ssl) < 0))
        {
            SSL_shutdown(ssl);
            SSL_free(ssl);
            netClose(conn);
            continue;
        }    

        if (pthread_create(&thread[threadCount], NULL, threadHTTPS, (void*)&args) != 0)
        {
            printLog(LOG_WARNING, "Can`t create thread.\n");
            continue;
        }
        threadCount++;
    }

    for (int i = 0; i < threadCount; ++i)
    {
        pthread_join(thread[i], NULL);
    }

    SSL_CTX_free(ctx);
    cleanupOpenSSL();
    return 0;
}

char* codeToStr(httpCode code)
{
    switch (code)
    {
        case HTTP_CODE_BAD_REQUEST:
            return " 400 Bad Request";
            break;
        case HTTP_CODE_NOT_FOUND:
            return " 404 Not Found";
            break;
        case HTTP_CODE_NOT_IMPLEMENTED:
            return " 501 Not Implemented";
            break;
        case HTTP_CODE_OK:
            return " 200 OK";
            break;
        case HTTP_CODE_FORBIDDEN:
            return " 403 Forbidden";
            break;
        default:
            return " 400 Bad Request";
            break;
    }
}

void* threadHTTPS(void *ptr)
{
    struct httpsArg *args = (struct httpsArg*)ptr;
    httpRequest httpReq;
    char buffer[BUFSIZE] = {0};

    while (1)
    {
        int number = netRecv(args->ssl, buffer, BUFSIZE);
        if (number <= 0)
        {
            printLog(LOG_DEBUG, "httpListen: recieved 0 bytes %s\n", strerror(errno));
            break;
        }
        parseRequest(&httpReq, buffer, number);

        httpResponse httpRes;
        createResponse(&httpReq, &httpRes, args->httpStruct);
        sendResponse(&httpRes, args->conn, args->ssl);
        
        memset(&httpRes, 0, sizeof(httpRes));
        strcpy(httpRes.filePath, "");
        if (httpReq.isKeepAlive == 0)
        {
            break;
        }
    }
    
    SSL_shutdown(args->ssl);
    SSL_free(args->ssl);
    netClose(args->conn);
}

void sendChunked(int fp, int conn, httpResponse *response, SSL *ssl)
{
    char str[BUFSIZE];
    int buflen;
    
    char chunk[CHUNK_SIZE];
    char hex[CONTENT_SIZE];
    while (buflen = read(fp, chunk, CHUNK_SIZE - 1))
    {
        if (buflen < 0)
        {
            printLog(LOG_ERROR, "sendResponse: %s\n", strerror(errno));
            return;
        }
        chunk[buflen] = '\0';
        sprintf(hex, "%x", buflen);
        strcpy(str, hex);
        strcat(str, "\r\n");
        strcat(str, chunk);
        strcat(str, "\r\n");
        netSend(ssl, str, strlen(str));
    }
       
    strcpy(str, "0\r\n\r\n");
    netSend(ssl, str, strlen(str));
}

void sendNotChunked(int fp, int conn, char *buffer, httpResponse *response, SSL *ssl)
{
    char str[BUFSIZE];
    int buflen;

    buflen = read(fp, str, BUFSIZE);
    if (buflen < 0)
    {
        printLog(LOG_ERROR, "sendResponse: %s\n", strerror(errno));
        return;
    }

    strcat(buffer, str);
    netSend(ssl, buffer, BUFSIZE);
}