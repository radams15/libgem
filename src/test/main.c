#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Gem.h"

#define HEADER_CHARS "***************"
#define MAX_LINKS 256
#define BITMAP_EMPTY (-1)

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
    TokList_t toks = get_page(page);

    size_t links[MAX_LINKS];
    bitmap_init(links, MAX_LINKS);

    for(int i=0 ; i<toks.length ; i++){
        Token_t tok = toks.data[i];

        switch (tok.type) {
            case TOKEN_HEADER: {
                printf("%.*s %s %.*s\n", tok.header_level, HEADER_CHARS, tok.data, tok.header_level, HEADER_CHARS);
            }break;

            case TOKEN_LIST_ITEM: {
                printf(" - %s\n", tok.data);
            }break;

            case TOKEN_TEXT: {
                printf("%s\n", tok.data);
            }break;

            case TOKEN_LINK: {
                printf("(%d) [%s = %s]\n", bitmap_used(links), tok.link_text, tok.link_page.page);
                bitmap_add(links, (size_t) &tok);
            }break;

            case TOKEN_QUOTE: {
                printf("''%s''\n", tok.data);
            }break;

            case TOKEN_PREFORMAT: {
                printf("[ %s ]\n", tok.data);
            }break;

            case TOKEN_NEWLINE:
                printf("\n\n");
                break;

            case TOKEN_EOF:
                break;
        }
    }

    printf("\t=> ");
    char entry[256];
    fgets(entry, sizeof entry, stdin);

    if(entry[0] == 'c'){
        int index = atoi(entry + 1);
        Page_t to_go = ((Token_t*)links[index])->link_page;
        render_page(to_go);
    }else if(entry[0] == 'g'){
        char* url = entry+1;
        if(url[strlen(url)-1] == '\n' || url[strlen(url)-1] == '\r'){
            url[strlen(url)-1] = 0;
        }

        Page_t page = parse_url(url);

        render_page(page);
    }

    toklist_free(&toks);
}

int main () {
    Page_t homepage = {"gemini", "gemini.circumlunar.space", ""};

    render_page(homepage);
}