#include <stdio.h>

#include "Gem.h"

#define HEADER_CHARS "***************"

int main () {
    TokList_t* toks = get_page("gemini.circumlunar.space", "");

    for(int i=0 ; i<toks->length ; i++){
        Token_t* tok = toks->data[i];

        if(tok->type == TOKEN_LINK) {

        }

        switch (tok->type) {
            case TOKEN_HEADER: {
                HeaderToken_t* lnk = tok;
                printf("%.*s %s %.*s\n", lnk->level, HEADER_CHARS, lnk->token.data, lnk->level, HEADER_CHARS);
            }break;

            case TOKEN_LIST_ITEM: {
                printf(" - %s\n", tok->data);
            }break;

            case TOKEN_TEXT: {
                printf("%s\n", tok->data);
            }break;

            case TOKEN_LINK: {
                LinkToken_t* lnk = tok;
                printf("[%s = %s]\n", lnk->text, lnk->url);
            }break;
        }
    }
}