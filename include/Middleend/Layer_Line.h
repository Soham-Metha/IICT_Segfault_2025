#ifndef LINE_LAYER_MIDDLEEND
#define LINE_LAYER_MIDDLEEND

#include <Middleend/Layer_Statement.h>

typedef struct BlockScope BlockScope;

struct BlockScope {
    Var_IR var_defs[128];
    int var_def_cnt;
    BlockScope *prev;
};

#endif