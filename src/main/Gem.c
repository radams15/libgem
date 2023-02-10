//
// Created by rhys on 11/05/22.
//

#include "Gem.h"
#include "Socket.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#ifdef __linux
#include <netdb.h>
#include <arpa/inet.h>
#endif

const char* ip_lookup(const char* host);

#ifdef __linux
const char* ip_lookup(const char* host){
    char out[1024];

    struct hostent *hent;
    struct in_addr **addr_list;
    int i;
    if ( (hent = gethostbyname( host ) ) == NULL){
        herror("gethostbyname error");
        return NULL;

    }
    addr_list = (struct in_addr **) hent->h_addr_list;

    for(i = 0; addr_list[i] != NULL; i++) {
        strcpy(&out , inet_ntoa(*addr_list[i]));
    }

    return strdup(out);
}
#endif

TokList_t *get_page(const char *base, const char *page) {
    const char* ip = ip_lookup(base);

    Socket_t* sock = socket_new(ip, 1965);

    char* page_full = (char*) calloc(strlen(base) + strlen(page) + 16, sizeof(char));
    sprintf(page_full, "gemini://%s/%s\r\n", base, page);

    socket_write(sock, page_full, strlen(page_full));
    Response_t* header = socket_read(sock, 1024); //todo: deal with input codes

    Response_t* resp = socket_read_all(sock);

    printf("%s\n", resp->content);

    TokList_t* toks = parse(resp->content, page_full);

    response_free(resp);

    socket_free(sock);

    return toks;
}
