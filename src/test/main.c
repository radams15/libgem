#include <stdio.h>

#include "Gem.h"

int main () {
    TokList_t* toks = get_page("gemini.circumlunar.space", "docs/gemtext.gmi");

    for(int i=0 ; i<toks->length ; i++){
        Token_t* tok = toks->data[i];
        printf("Type: %s\n", tok->data);
    }
}