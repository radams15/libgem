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

typedef struct Token {
    TokenType_t type;
    char* data;
    int length;
} Token_t;

typedef struct HeaderToken{
    Token_t token;
    int level;
} HeaderToken_t;

typedef struct LinkToken{
    Token_t token;
    char* url;
    char* text;
} LinkToken_t;

typedef struct PreToken{
    Token_t token;
    char* alt;
} PreToken_t;

typedef struct TokList {
    Token_t** data;
    int length;
} TokList_t;

TokList_t* parse(const char* text, const char* current_page);

#ifdef __cplusplus
};
#endif

#endif //LIBGEM_PARSER_H
