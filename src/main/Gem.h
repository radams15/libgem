//
// Created by rhys on 11/05/22.
//

#ifndef LIBGEM_GEM_H
#define LIBGEM_GEM_H

#include "Parser.h"

#ifdef __cplusplus
extern "C" {
#endif

TokList_t* get_page(const char* base, const char* page);

#ifdef __cplusplus
};
#endif

#endif //LIBGEM_GEM_H
