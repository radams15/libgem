//
// Created by rhys on 11/05/22.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Parser.h"

#define TRY_FREE(a) a != NULL? free(a):1

const char* dups(const char* in) {
    char* out = calloc(strlen(in)+1, sizeof(char));

    strncpy(out, in, strlen(in));
    out[strlen(in)] = 0;

    return out;
}

const char* subnstr(const char* in, int start, int length) {
    char* out = calloc(length+1, sizeof(char));

    snprintf(out, length, "%s", in+start);

    return out;
}

const char* substr(const char* in, int start) {
    return subnstr(in, start, strlen(in)-start+1);
}

Token_t list_item(const char* line){
    return (Token_t) {
        .type = TOKEN_LIST_ITEM,
        .data = (char*) dups(line),
        .length = strlen(line)
    };
}

Token_t quote(const char* line){
    return (Token_t) {
            .type = TOKEN_QUOTE,
            .data = (char*) dups(line),
            .length = strlen(line)
    };
}

Token_t header(const char* line) {
    Token_t tok = (Token_t) {
            .type = TOKEN_HEADER,
            .length = strlen(line)
    };

    const char* chars = line;
    for(tok.header_level=0 ; *chars++ == '#' ; tok.header_level++){}

    tok.length -= tok.header_level;
    tok.data = (char*) substr(line, tok.header_level);

    return tok;
}

const char* get_page_proto(const char* url) {
    int url_len = strlen(url);

    int end;
    for(end=0 ; url[end] != ':' && url[end+1] != '/' && url[end+2] != '/' && end+2 < url_len ; end++);
    end++;

    char* out = (char*) calloc(end+1, sizeof(char));
    strncpy(out, url, end);

    if(strncmp(out, url, strlen(out)) != 0){ // There was no protocol.
        free(out);
        return dups(""); // Output no protocol if none defined.
    }

    return out;
}

char* get_page_base(const char* page) {
    const char* proto = get_page_proto(page);

    char* copy_mut = (char*) substr(page, strlen(proto));

    free(proto);

    strtok(copy_mut, "/");
    char*base = strtok(NULL, "/");

    char* out = dups(base);
    free(copy_mut);

    return out;
}

int starts_with_proto(const char* line) {
    const char* proto = get_page_proto(line);

    int out = strcmp("https", proto) == 0
        || strcmp("http", proto) == 0
        || strcmp("gopher", proto) == 0
        || strcmp("gemini", proto) == 0
        || strcmp("ftp", proto) == 0;

    free((void*) proto);

    return out;
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

    char* out = subnstr(in, leading, strlen(in)-leading-trailing+1);

    free((void*) in);

    return out;
}

Page_t parse_url(const char* url) {
    const char* proto = get_page_proto(url);
    const char* base = get_page_base(url);
    const char* page = dups(url + strlen(proto) + 3 + strlen(base));

    return (Page_t){
        proto,
        base,
        page
    };
}

Token_t link(const char* line, const char* current_page){
    Token_t tok = (Token_t) {
            .type = TOKEN_LINK,
            .data = (char *) dups(line),
            .length = strlen(line)
    };

    if(current_page[strlen(current_page)-2] == '\r'){
        ((char*)current_page)[strlen(current_page)-3] = 0;
    }

    char* data_cpy = (char*)dups(tok.data);
    char* url = strtok(data_cpy, "\t");
    if(url == NULL){
        url = strtok(data_cpy, " ");
    }
    char* text = strtok(NULL, "\t");
    if(text == NULL){
        url = strtok(data_cpy, " ");
        text = strtok(NULL, " ");
    }

    url = (char*)dups(url);

    if(text == NULL){
        text = (char*) "";
    }

    tok.link_text = (char*) dups(text);

    const char* base = get_page_base(current_page);

    url = (char*) strip((const char*) url);
    base = strip(base);

    if(! starts_with_proto(url)){ // Does not start with protocol, e.g. internal
        char* current_page_baseless = (char*) dups((char*) current_page + strlen("gemini://") + strlen(base));
        char* new_page = (char*) calloc(strlen(current_page_baseless) + strlen(url) + 2, sizeof(char));
        sprintf(new_page, "%s/%s", current_page_baseless, url);

        free(current_page_baseless);
        tok.link_page = (Page_t){
                .proto = dups("gemini"),
                .base = dups(base),
                .page = new_page
        };
    } else{
        tok.link_page = parse_url(url);
    }

    free((void*) base);
    free((void*) url);

    free(data_cpy);

    return tok;
}

Token_t pre(const char* line){
    return (Token_t) {
            .type = TOKEN_PREFORMAT,
            .data = (char*) dups(line),
            .length = strlen(line)
    };
}

Token_t text(const char* line){
    return (Token_t) {
            .type = TOKEN_TEXT,
            .data = (char*) dups(line),
            .length = strlen(line)
    };;
}

int begins(const char* haystack, const char* needle){
    return strncmp(haystack, needle, strlen(needle)) == 0;
}

Token_t scan(const char* line, const char* current_page){
    if(strcmp(line, "\r\n") == 0){
        return (Token_t) {
                TOKEN_NEWLINE,
                NULL,
                0
        };
    }else if(begins(line, "```")){
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

    return (Token_t){
        .type = TOKEN_EOF
    };
}

void append_pre(const char* line, Token_t* tok){
    tok->length += strlen(line)+2;

    tok->data = (char*) realloc(tok->data, tok->length*sizeof(char));

    strcat(tok->data, dups(line));
    strcat(tok->data, "\n");
}

void toklist_append(TokList_t* list, Token_t tok){
    list->length++;
    list->data = (Token_t*) realloc(list->data, list->length*sizeof(Token_t));

    list->data[list->length-1] = tok;
}

void token_free(Token_t* tok) {
    /*
    TokenType_t type;
    char* data;
    int length;
    int header_level;
    Page_t link_page;
    char* link_text;
    char* pre_alt;
     */

    TRY_FREE(tok->data);
    TRY_FREE(tok->link_text);
    TRY_FREE(tok->pre_alt);

    TRY_FREE((void*) tok->link_page.proto);
    TRY_FREE((void*) tok->link_page.page);
    TRY_FREE((void*) tok->link_page.base);
}

void toklist_free(TokList_t* list) {
    for(int i=0 ; i<list->length ; i++) {
        token_free(&(list->data[i]));
    }

    free(list->data);
}

TokList_t parse_page(const char* text, const char* current_page) {
    TokList_t out;
    out.data = (Token_t*) malloc(1);
    out.length = 0;

    int in_pre = 0;
    Token_t tok;
    char* line;
    char* text_mut = (char*)dups(text);

    line = text_mut;
    while (line != NULL) {
        char* next_line = strchr(line, '\n');
        if(next_line) *next_line = '\0';

        if(in_pre){
            if(begins(line, "```")){
                in_pre = 0;
                tok.pre_alt = (char*) substr(line, 3);
            }else{
                append_pre(line, &tok);
                goto end;
            }
        }else {
            tok = scan(line, current_page);

            if (tok.type == TOKEN_EOF) {
                goto end;
            }

            if(tok.type == TOKEN_PREFORMAT){
                in_pre = 1;
                goto end;
            }
        }

        toklist_append(&out, tok);

end:
        if(next_line) *next_line = '\n';
        line = next_line? (next_line+1) : NULL;
    }

    free(text_mut);

    return out;
}
