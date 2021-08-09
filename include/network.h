#ifndef NETWORK_H
#define NETWORK_H

#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

/**
 * @brief Close socket
 * 
 * @param conn socket
 * @return exit code 
 */
int netClose(int conn);

/**
 * @brief send data to socket
 * 
 * @param ssl ssl object
 * @param buffer data
 * @param size size of buffer
 * @return exit code
 */
int netSend(SSL *ssl, char* buffer, int size);
/**
 * @brief receiving data from socket
 * 
 * @param ssl ssl object
 * @param buffer data
 * @param size size of buffer
 * @return exit code
 */
int netRecv(SSL *ssl, char* buffer, int size);

/**
 * @brief accept connection
 * 
 * @param listener socket
 * @return exit code 
 */
int netAccept(int listener);
/**
 * @brief set socket for listening
 * 
 * @param address ip address
 * @param port port for listening
 * @return exit code
 */
int netListen(char* address, int port);

#endif
