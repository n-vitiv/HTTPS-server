#ifndef SSL_H
#define SSL_H

#include <openssl/ssl.h>
#include <openssl/err.h>

/**
 * @brief initialization of openssl
 * 
 */
void initOpenSSL();

/**
 * @brief cleaning openssl
 * 
 */
void cleanupOpenSSL();

/**
 * @brief Create a Context object
 * 
 * @return SSL object 
 */
SSL_CTX *createContext();

/**
 * @brief Configuration of SSL object
 * 
 * @param ctx SSL object which will be configured
 */
void configureContext(SSL_CTX *ctx);

/**
 * @brief set ssl with socket
 * 
 * @param ctx ssl object
 * @param sock socket
 * @return setted ssl 
 */
SSL *setSSL(SSL_CTX *ctx, int sock);

#endif
