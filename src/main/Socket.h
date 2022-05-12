//
// Created by rhys on 11/05/22.
//

#ifndef LIBGEM_SOCKET_H
#define LIBGEM_SOCKET_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Socket Socket_t;

typedef struct Response {
    char* content;
    int length;

    int error;
} Response_t;

Socket_t* socket_new(const char* ip, int port);

Response_t* socket_read(Socket_t* sock, int len);
Response_t* socket_read_all(Socket_t* sock);
int socket_write(Socket_t* sock, const char* buf, int len);

void response_free(Response_t* resp);

void socket_free(Socket_t* sock);

#ifdef __cplusplus
};
#endif

#endif //LIBGEM_SOCKET_H
