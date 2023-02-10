#include <stdio.h>
#include <stdlib.h>

#include "Gem.h"

#define HEADER_CHARS "***************"
#define MAX_LINKS 256
#define BITMAP_EMPTY -1

void bitmap_add(size_t* bitmap, size_t value){
    size_t* it;
    for(it=bitmap ; *it!=BITMAP_EMPTY ; ++it);
    *it = value;
}

void bitmap_init(size_t* bitmap, size_t length){
    for(size_t* it=bitmap; it!=bitmap+length ; ++it) *it=BITMAP_EMPTY;
}

int bitmap_used(size_t* bitmap){
    size_t* it;
    for(it=bitmap ; *it!=BITMAP_EMPTY ; ++it);
    return it-bitmap;
}

void render_page(Page_t page){
    TokList_t* toks = get_page(page);

    size_t links[MAX_LINKS];
    bitmap_init(links, MAX_LINKS);

    for(int i=0 ; i<toks->length ; i++){
        Token_t* tok = toks->data[i];

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
                printf("(%d) [%s = %s]\n", bitmap_used(links), lnk->text, lnk->page.page);
                bitmap_add(links, (size_t) lnk);
            }break;

            case TOKEN_QUOTE: {
                printf("''%s''\n", tok->data);
            }break;

            case TOKEN_PREFORMAT: {
                PreToken_t *pre = tok;
                printf("[ %s ]\n", tok->data);
            }break;

            case TOKEN_NEWLINE:
                printf("\n\n");
                break;

            case TOKEN_EOF:
                break;
        }
    }

    printf("Bitmap is %d long\n", bitmap_used(links));

    printf("\t=> ");
    char entry[256];
    fgets(entry, sizeof entry, stdin);

    if(entry[0] == 'c'){
        int index = atoi(entry + 1);
        Page_t to_go = ((LinkToken_t*)links[index])->page;
        render_page(to_go);
    }
}

int main () {
    Page_t homepage = {"gemini", "gemini.circumlunar.space", "docs/"};

    render_page(homepage);
}