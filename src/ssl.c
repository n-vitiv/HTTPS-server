#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include "../include/ssl.h"
#include "../include/logger.h"

void initOpenSSL()
{
    SSL_load_error_strings();	
    OpenSSL_add_ssl_algorithms();    
}

void cleanupOpenSSL()
{
    EVP_cleanup();
}

SSL_CTX *createContext()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = SSLv23_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) 
    {
        printLog(LOG_ERROR, "Can`t create SSL context.\n");
        return NULL;
    }

    return ctx;
}

void configureContext(SSL_CTX *ctx)
{
    SSL_CTX_set_ecdh_auto(ctx, 1);
    char *crtpath = canonicalize_file_name("keys/localhost.crt");
    //set key and certificate
    if (SSL_CTX_use_certificate_file(ctx, crtpath, SSL_FILETYPE_PEM) <= 0)
    {
        printLog(LOG_ERROR, "Can`t set certificate file.\n");
        ERR_print_errors_fp(stderr);
        goto err1;
    }
    char *keypath = canonicalize_file_name("keys/localhost.key");
    if (SSL_CTX_use_PrivateKey_file(ctx, keypath, SSL_FILETYPE_PEM) <= 0 )
    {
        printLog(LOG_ERROR, "Can`t set private key file.\n");
        ERR_print_errors_fp(stderr);
        goto err2;
    }
    return;
err2:
    free(keypath);
err1:
    free(crtpath);
    exit(1);
}

SSL *setSSL(SSL_CTX *ctx, int sock)
{
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);

    return ssl;
}