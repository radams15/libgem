//
// Created by rhys on 11/05/22.
//

#include <stdio.h>
#include <memory.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <unistd.h>

#include "Socket.h"

#define CHK_NULL(x) if ((x)==NULL){ fprintf(stderr, "Null ptr detected!\n");exit (1); }
#define CHK_ERR(err,s) if ((err)==-1) { perror(s); exit(1); }
#define CHK_SSL(err) if ((err)==-1) { ERR_print_errors_fp(stderr); exit(1); }

#define CHUNK_SIZ 1024

struct Socket {
    int sd;
    SSL_CTX* ctx;
    SSL* ssl;
};

Socket_t* socket_new(const char* ip, int port) {
    Socket_t* out = malloc(sizeof(Socket_t));

    OpenSSL_add_ssl_algorithms();
    const SSL_METHOD* meth = TLS_client_method();
    SSL_load_error_strings();
    out->ctx = SSL_CTX_new (meth);                        CHK_NULL(out->ctx);

    out->sd = socket (AF_INET, SOCK_STREAM, 0);
    CHK_ERR(out->sd, "socket");


    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family      = AF_INET;
    sa.sin_addr.s_addr = inet_addr (ip);
    sa.sin_port        = htons     (port);

    int err = connect(out->sd, (struct sockaddr*) &sa,sizeof(sa));
    CHK_ERR(err, "connect");


    out->ssl = SSL_new (out->ctx);
    CHK_NULL(out->ssl);

    SSL_set_fd (out->ssl, out->sd);
    err = SSL_connect (out->ssl);
    CHK_SSL(err);

#ifdef DEBUG
    char* str;
    X509* server_cert;

    printf ("SSL connection using %s\n", SSL_get_cipher (out->ssl));

    server_cert = SSL_get_peer_certificate (out->ssl);       CHK_NULL(server_cert);
    printf ("Server certificate:\n");

    str = X509_NAME_oneline (X509_get_subject_name (server_cert),0,0);
    CHK_NULL(str);
    printf ("\t subject: %s\n", str);
    OPENSSL_free (str);

    str = X509_NAME_oneline (X509_get_issuer_name  (server_cert),0,0);
    CHK_NULL(str);
    printf ("\t issuer: %s\n", str);
    OPENSSL_free (str);


    X509_free (server_cert);
#endif


    return out;
}

Response_t* socket_read(Socket_t *sock, int len) {
    Response_t* out = malloc(sizeof(Response_t));

    out->content = malloc(len*sizeof(char));

    out->length = SSL_read(sock->ssl, (void*) out->content, len-1);

    out->error = out->length!=-1? 0 : out->length;

    if(out->error){
        ERR_print_errors_fp(stderr);
        exit(1);
    }

    out->content[out->length] = 0;

    return out;
}

void socket_free(Socket_t *sock) {
    SSL_shutdown (sock->ssl);  /* send SSL/TLS close_notify */

    close(sock->sd);
    SSL_free(sock->ssl);
    SSL_CTX_free(sock->ctx);

    free(sock);
}

int socket_write(Socket_t* sock, const char* buf, int len) {
    int err = SSL_write (sock->ssl, buf, len);

    return err;
}

Response_t* socket_read_all(Socket_t *sock) {
    Response_t* out = malloc(sizeof(Response_t));

    out->content = calloc(1, sizeof(char));
    out->length = 0;

    char tmp[CHUNK_SIZ];

    int length;
    while(1) {
        length = SSL_read(sock->ssl, (void *) tmp, CHUNK_SIZ);

        if(length == 0){
            break;
        }

        out->length += length;

#ifdef DEBUG
        printf("New length: %d, Total length: %d\n", length, out->length);
#endif

        out->content = realloc(out->content, (out->length+1)*sizeof(char));
        strncat(out->content, tmp, length);
    }

    return out;
}

void response_free(Response_t *resp) {
    free(resp->content);
    free(resp);
}
