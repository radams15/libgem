#include <stdio.h>

#include "Gem.h"

int main () {
    TokList_t* toks = get_page("gemini.circumlunar.space", "docs/gemtext.gmi");

    for(int i=0 ; i<toks->length ; i++){
        Token_t* tok = toks->data[i];

        if(tok->type == TOKEN_LINK) {
            LinkToken_t* lt = (LinkToken_t*) tok;
            printf("To: %s (%s)\n", lt->url, lt->base);
        }
    }
}