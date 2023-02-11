//
// Created by rhys on 11/05/22.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Parser.h"

Token_t* list_item(const char* line){
    Token_t* tok = (Token_t*) malloc(sizeof(Token_t));
    tok->type = TOKEN_LIST_ITEM;

    tok->data = strdup(line);
    tok->length = strlen(line);

    return tok;
}

Token_t* quote(const char* line){
    Token_t* tok = (Token_t*) malloc(sizeof(Token_t));
    tok->type = TOKEN_QUOTE;

    tok->data = strdup(line);
    tok->length = strlen(line);

    return tok;
}

Token_t* header(const char* line) {
    HeaderToken_t* headertok = (HeaderToken_t*) malloc(sizeof(HeaderToken_t));
    Token_t* tok = (Token_t*) headertok;

    tok->type = TOKEN_HEADER;
    tok->data = strdup(line);
    tok->length = strlen(line);

    char* chars = tok->data;
    for(headertok->level=0 ; *chars++ == '#' ; headertok->level++){}

    tok->length -= headertok->level;
    tok->data += headertok->level;

    return tok;
}

const char* get_page_proto(const char* url) {
    int url_len = strlen(url);

    int end;
    for(end=0 ; url[end] != ':' && url[end+1] != '/' && url[end+2] != '/' && end+2 < url_len ; end++);
    end++;

    char* out = (char*) calloc(end+1, sizeof(char));
    strncpy(out, url, end);

    if(strncmp(out, url, strlen(out)) == 0){ // There was no protocol.
        free(out);
        return strdup("gemini"); // Assume gemini protocol.
    }

    return out;
}

char* get_page_base(const char* page) {
    const char* proto = get_page_proto(page);

    char* copy_mut = strdup(page);

    copy_mut += strlen(proto) + 3;
    free(proto);
    strtok(copy_mut, "/");

    char* out = strdup(copy_mut);
    free(copy_mut);

    return out;
}

int starts_with_proto(const char* line) {
    const char* proto = get_page_proto(line);

    return strcmp("https", proto) == 0
        || strcmp("http", proto) == 0
        || strcmp("gopher", proto) == 0
        || strcmp("gemini", proto) == 0
        || strcmp("ftp", proto) == 0;
}

const char* strip(const char* in){
    int leading = 0;
    for (char c = *in; c == ' ' || c == '\t'; ++c) { leading++; }

    int trailing = 0;
    for(int i=strlen(in)-1 ; i>= 0 ; i++){
        if(in[i] == ' ' || in[i] == '\t'){
            trailing++;
        }else{
            break;
        }
    }

    char* out = strndup(in, strlen(in)-trailing);
    out += leading;

    free((void*) in);

    return out;
}

Page_t parse_url(const char* url) {
    const char* proto = get_page_proto(url);
    const char* base = get_page_base(url);
    const char* page = url + strlen(proto) + 3 + strlen(base);

    return (Page_t){
        proto,
        base,
        page
    };
}

Token_t* link(const char* line, const char* current_page){
    LinkToken_t* linktok = (LinkToken_t*) malloc(sizeof(LinkToken_t));
    Token_t* tok = (Token_t*) linktok;

    if(current_page[strlen(current_page)-2] == '\r'){
        ((char*)current_page)[strlen(current_page)-3] = 0;
    }

    tok->type = TOKEN_LINK;
    tok->data = strdup(line);
    tok->length = strlen(line);

    char* data_cpy = strdup(tok->data);
    char* url = strtok(data_cpy, "\t");
    if(url == NULL){
        url = strtok(data_cpy, " ");
    }
    char* text = strtok(NULL, "\t");
    if(text == NULL){
        url = strtok(data_cpy, " ");
        text = strtok(NULL, " ");
    }

    url = strdup(url);

    if(text == NULL){
        text = (char*) "";
    }

    linktok->text = strdup(text);

    const char* base = get_page_base(current_page);

    url = (char*) strip((const char*) url);
    base = strip(base);

    if(! starts_with_proto(url)){ // Does not start with protocol, e.g. internal
        char* current_page_baseless = strdup((char*) current_page + strlen("gemini://") + strlen(base));
        char* new_page = (char*) calloc(strlen(current_page_baseless) + strlen(url) + 2, sizeof(char));
        sprintf(new_page, "%s/%s", current_page_baseless, url);

        free(current_page_baseless);
        linktok->page = (Page_t){
                .proto = "gemini",
                .base = strdup(base),
                .page = new_page
        };
    } else{
        linktok->page = parse_url(url);
    }

    free((void*) base);

    free(data_cpy);

    return tok;
}

Token_t* pre(const char* line){
    PreToken_t* pretok = (PreToken_t*) malloc(sizeof(PreToken_t));
    Token_t* tok = (Token_t*) pretok;

    tok->type = TOKEN_PREFORMAT;
    tok->data = strdup(line);
    tok->length = strlen(line);

    return tok;
}

Token_t* text(const char* line){
    Token_t* tok = (Token_t*) malloc(sizeof(Token_t));
    tok->type = TOKEN_TEXT;

    tok->data = strdup(line);
    tok->length = strlen(line);

    return tok;
}

int begins(const char* haystack, const char* needle){
    return strncmp(haystack, needle, strlen(needle)) == 0;
}

Token_t* scan(const char* line, const char* current_page){
    /*if(line == "\n"){
        return new Token_t {
                TOKEN_NEWLINE,
                NULL,
                0
        };
    }else */
    if(begins(line, "```")){
        return pre(line+3);
    }else if(begins(line, "*")){
        return list_item(line+1);
    }else if(begins(line, "#")){
        return header(line);
    }else if(begins(line, "=>")){
        return link(line+2, current_page);
    }else if(begins(line, ">")){
        return quote(line+1);
    }else if(strlen(line) > 0){
        return text(line);
    }

    return NULL;
}

void append_pre(const char* line, PreToken_t* pretok){
    Token_t* tok = (Token_t*) pretok;

    tok->length += strlen(line)+2;

    tok->data = (char*) realloc(tok->data, tok->length*sizeof(char));

    strcat(tok->data, strdup(line));
    strcat(tok->data, "\n");
}

void toklist_append(TokList_t* list, Token_t* tok){
    list->length++;
    list->data = (Token_t**) realloc(list->data, list->length*sizeof(Token_t*));

    list->data[list->length-1] = tok;
}

TokList_t* parse_page(const char* text, const char* current_page) {
    TokList_t* out = (TokList_t*) malloc(sizeof(TokList_t));
    out->data = (Token_t**) malloc(1);
    out->length = 0;

    int in_pre = 0;
    Token_t* tok;
    char* line;
    char* text_mut = strdup(text);

    printf("Raw: %s\n\n\n", text);

    line = strtok(text_mut, "\r\n");
    while (line != NULL) {
        printf("Line: %s\n", line);
        if(in_pre){
            if(begins(line, "```")){
                in_pre = 0;
                ((PreToken_t*)tok)->alt = strdup(line+3);
            }else{
                append_pre(line, (PreToken_t*) tok);
                goto end;
            }
        }else {
            tok = scan(strdup(line), current_page);

            if (tok == NULL) {
                goto end;
            }

            if(tok->type == TOKEN_PREFORMAT){
                in_pre = 1;
                goto end;
            }
        }

        toklist_append(out, tok);

end:
        line = strtok(NULL, "\r\n");
    }

    return out;
}
