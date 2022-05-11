#include <stdio.h>
#include <string.h>

#include "Socket.h"
#include "Parser.h"

#define READ() resp = socket_read(sock, 1024);printf("Response: %s\n", resp->content);response_free(resp);
#define WRITE(str) socket_write(sock, str, strlen(str));

int main () {
    Socket_t* sock = socket_new("168.235.111.58", 1965);

    Response_t* resp;

    WRITE("gemini://gemini.circumlunar.space/docs/gemtext.gmi\r\n");
    READ();
    resp = socket_read_all(sock);

    TokList_t* toks = parse(resp->content);

    for(int i=0 ; i<toks->length ; i++){
        Token_t* tok = toks->data[i];
        printf("Type: %d\n", tok->type);
    }

    response_free(resp);

    socket_free(sock);
}