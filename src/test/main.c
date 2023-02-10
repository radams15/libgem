#include <stdio.h>

#include "Gem.h"

#define HEADER_CHARS "***************"

void render_page(Page_t page){
    TokList_t* toks = get_page(page);

    for(int i=0 ; i<toks->length ; i++){
        Token_t* tok = toks->data[i];

        /*if(tok->type == TOKEN_LINK) {
            LinkToken_t* lnk = tok;
            printf("[%s = %s:%s:%s]\n", lnk->text, lnk->page.proto, lnk->page.base, lnk->page.page);
        }*/

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
                printf("[%s = %s]\n", lnk->text, lnk->page.page);
            }break;

            case TOKEN_QUOTE:
                printf("''%s''\n", tok->data);
                break;

            case TOKEN_PREFORMAT:
                break;

            case TOKEN_NEWLINE:
                printf("\n\n");
                break;

            case TOKEN_EOF:
                break;
        }
    }
}

int main () {
    Page_t homepage = {"gemini", "gemini.circumlunar.space", ""};

    render_page(homepage);
}