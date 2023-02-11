//
// Created by rhys on 11/05/22.
//

#ifndef LIBGEM_PARSER_H
#define LIBGEM_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum TokenType {
    TOKEN_TEXT,
    TOKEN_HEADER,
    TOKEN_LINK,
    TOKEN_LIST_ITEM,
    TOKEN_QUOTE,
    TOKEN_PREFORMAT,
    TOKEN_NEWLINE,
    TOKEN_EOF
} TokenType_t;

typedef struct Page {
    const char* proto;
    const char* base;
    const char* page;
} Page_t;

typedef struct Token {
    TokenType_t type;
    char* data;
    int length;
    int header_level;
    Page_t link_page;
    char* link_text;
    char* pre_alt;
} Token_t;

typedef struct TokList {
    Token_t* data;
    int length;
} TokList_t;

TokList_t parse_page(const char* text, const char* current_page);
Page_t parse_url(const char* url);

void toklist_free(TokList_t* list);

#ifdef __cplusplus
};
#endif

#endif //LIBGEM_PARSER_H
