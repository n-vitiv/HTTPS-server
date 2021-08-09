#ifndef HTTP_H
#define HTTP_H

#include "ssl.h"

#define FILE_PATH_SIZE 2048
#define METHOD_SIZE 8
#define PROTOCOL_VERSION_SIZE 10
#define IP_SIZE 40
#define ROOT_FOLDER_SIZE 2048
#define DEFAULT_DOCUMENT_SIZE 256

typedef enum
{
    HTTP_CODE_OK = 200,
    HTTP_CODE_BAD_REQUEST = 400,
    HTTP_CODE_NOT_FOUND = 404,
    HTTP_CODE_NOT_IMPLEMENTED = 501,
    HTTP_CODE_FORBIDDEN = 403
} httpCode;

/**
 * @brief struct for saving data of http request
 * 
 */
typedef struct 
{
    char filePath[FILE_PATH_SIZE];
    char method[METHOD_SIZE];
    char protocolVersion[PROTOCOL_VERSION_SIZE];
    int isKeepAlive;
} httpRequest;

/**
 * @brief struct for saving data of http response
 * 
 */
typedef struct 
{
    char filePath[FILE_PATH_SIZE];
    httpCode code;
    int fileSize;
    char protocolVersion[PROTOCOL_VERSION_SIZE];
    int isChunked;
} httpResponse;

/**
 * @brief struct for saving main http data
 * 
 */
typedef struct 
{
    char ip[IP_SIZE];
    int port;
    char rootFolder[ROOT_FOLDER_SIZE];
    char defaultDocument[DEFAULT_DOCUMENT_SIZE];
} http;

/**
 * @brief initialization of http
 * 
 * @return http  
 */
http* httpInit();

/**
 * @brief closing http module
 * 
 * @param httpStruct struct which will be deinitializated
 */
void httpDeinit(http* httpStruct);

/**
 * @brief parsing http requests
 * 
 * @param request struct which will be filled
 * @param buffer buffer with http data
 * @param size size of buffer
 */
void parseRequest(httpRequest *request, char *buffer, int size);

/**
 * @brief Create a Response object
 * 
 * @param request struct with http request
 * @param response struct which will be filled 
 * @param httpStruct http struct with default params
 */
void createResponse(httpRequest *request, httpResponse *response, http *httpStruct);

/**
 * @brief send response object
 * 
 * @param response object which will be sent
 * @param conn socket
 */
void sendResponse(httpResponse *response, int conn, SSL *ssl);

/**
 * @brief set http for listening
 * 
 * @param httpStruct struct with default params
 * 
 * @return exit code
 */
int httpListen(http* httpStruct);

/**
 * @brief send chunked response
 * 
 * @param fp file descriptor
 * @param conn socket
 */
void sendChunked(int fp, int conn, httpResponse *response, SSL *ssl);

/**
 * @brief send not chunked response
 * 
 * @param fp file descriptor
 * @param conn socket
 * @param buffer buffer which will be sent
 */
void sendNotChunked(int fp, int conn, char *buffer, httpResponse *response, SSL *ssl);

#endif
