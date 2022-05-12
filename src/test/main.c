#include <stdio.h>

#include "Gem.h"

int main () {
    TokList_t* toks = get_page("gemini.circumlunar.space", "docs/gemtext.gmi");

    for(int i=0 ; i<toks->length ; i++){
        Token_t* tok = toks->data[i];

        if(tok->type == TOKEN_QUOTE) {
            printf("To: %s\n", tok->data);
        }
    }
}