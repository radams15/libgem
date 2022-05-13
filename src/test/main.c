#include <stdio.h>

#include "Gem.h"

int main () {
    TokList_t* toks = get_page("gemini.circumlunar.space", "");

    for(int i=0 ; i<toks->length ; i++){
        Token_t* tok = toks->data[i];

        if(tok->type == TOKEN_LINK) {
            LinkToken_t* lnk = tok;
            printf("To: %s\n", lnk->url);
        }
    }
}