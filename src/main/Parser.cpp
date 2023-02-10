//
// Created by rhys on 11/05/22.
//

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iostream>
#include <vector>

#include "Parser.h"

Token_t* list_item(std::string line){
    Token_t* tok = (Token_t*) malloc(sizeof(Token_t));
    tok->type = TOKEN_LIST_ITEM;

    tok->data = strdup(line.c_str()+1);
    tok->length = line.size()-1;

    return tok;
}

Token_t* quote(std::string line){
    Token_t* tok = (Token_t*) malloc(sizeof(Token_t));
    tok->type = TOKEN_QUOTE;

    tok->data = strdup(line.c_str()+1);
    tok->length = line.size()-1;

    return tok;
}

Token_t* header(std::string line){
    HeaderToken_t* headertok = (HeaderToken_t*) malloc(sizeof(HeaderToken_t));
    Token_t* tok = (Token_t*) headertok;

    tok->type = TOKEN_HEADER;
    tok->data = strdup(line.c_str());
    tok->length = line.size();

    char* chars = tok->data;
    for(headertok->level=0 ; *chars++ == '#' ; headertok->level++){}

    tok->length -= headertok->level;
    tok->data += headertok->level;

    return tok;
}

char* get_page_base(const char* page){
    int last_len = 0;

    char* copy_mut = strdup(page);

    char* ptr;

    strtok(copy_mut, "/");
    while(1){
        ptr = strtok(NULL, "/");

        if(ptr == NULL){
            break;
        }

        last_len = strlen(ptr);
    }

    int new_len = strlen(page) - (last_len - 1);

    char* out = (char*) calloc(new_len+1, sizeof(char));

    sprintf(out, "%.*s", new_len-1, page);
    out[new_len] = 0;

    free(copy_mut);

    return out;
}

bool str_starts(std::string needle, std::string haystack) {
    return haystack.find(needle) != -1;
}

int starts_with_proto(const char* line){
    std::string ls = std::string(line);

    return str_starts("https://", ls)
    || str_starts("http://", ls)
    || str_starts("gopher://", ls)
    || str_starts("gemini://", ls)
    || str_starts("ftp://", ls);
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

Token_t* link(std::string line, const char* current_page){
    LinkToken_t* linktok = (LinkToken_t*) malloc(sizeof(LinkToken_t));
    Token_t* tok = (Token_t*) linktok;

    //todo remove leading spaces

    tok->type = TOKEN_LINK;
    tok->data = strdup(line.c_str()+2);
    tok->length = line.size()-2;

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
        printf("%s = no proto\n", url);
        linktok->page = {
                .base = strdup(base),
                .page = strdup(url)
        };
    } else{
        fprintf(stderr, "Cannot interpret URL: '%s'\n", url);
        exit(1);
    }

    free((void*) base);

    free(data_cpy);

    return tok;
}

Token_t* pre(std::string line){
    PreToken_t* pretok = (PreToken_t*) malloc(sizeof(PreToken_t));
    Token_t* tok = (Token_t*) pretok;

    tok->type = TOKEN_PREFORMAT;
    tok->data = strdup(line.c_str()+3);
    tok->length = line.size()-3;

    return tok;
}

Token_t* text(std::string line){
    Token_t* tok = (Token_t*) malloc(sizeof(Token_t));
    tok->type = TOKEN_TEXT;

    tok->data = strdup(line.c_str());
    tok->length = line.size();

    return tok;
}

bool begins(std::string haystack, std::string needle){
    return haystack.find(needle) == 0;
}

Token_t* scan(std::string line, const char* current_page){
    if(line == "\n"){
        return new Token_t {
                TOKEN_NEWLINE,
                NULL,
                0
        };
    }else if(begins(line, "```")){
        return pre(line);
    }else if(begins(line, "*")){
        return list_item(line);
    }else if(begins(line, "#")){
        return header(line);
    }else if(begins(line, "=>")){
        return link(line, current_page);
    }else if(begins(line, ">")){
        return quote(line);
    }else if(line.length() > 0){
        return text(line);
    }

    return NULL;
}

void append_pre(std::string line, PreToken_t* pretok){
    Token_t* tok = (Token_t*) pretok;

    tok->length += line.size()+2;

    tok->data = (char*) realloc(tok->data, tok->length*sizeof(char));

    strcat(tok->data, strdup(line.c_str()));
    strcat(tok->data, "\n");
}

void toklist_append(TokList_t* list, Token_t* tok){
    list->length++;
    list->data = (Token_t**) realloc(list->data, list->length*sizeof(Token_t*));

    list->data[list->length-1] = tok;
}

TokList_t* parse(const char* text, const char* current_page) {
    TokList_t* out = (TokList_t*) malloc(sizeof(TokList_t));
    out->data = (Token_t**) malloc(1);
    out->length = 0;

    std::string in = (std::string) text;
    std::istringstream ss(in);
    std::string line;

    bool in_pre = false;
    Token_t* tok;

    while (getline(ss, line)) {
        if(in_pre){
            if(begins(line, "```")){
                in_pre = false;
                ((PreToken_t*)tok)->alt = strdup(line.c_str()+3);
            }else{
                append_pre(line, (PreToken_t*) tok);
                continue;
            }
        }else {
            tok = scan(line, current_page);

            if (tok == NULL) {
                continue;
            }

            if(tok->type == TOKEN_PREFORMAT){
                in_pre = true;
                continue;
            }
        }

        toklist_append(out, tok);

    }

    return out;
}
