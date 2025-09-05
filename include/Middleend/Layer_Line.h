#ifndef LINE_LAYER_MIDDLEEND
#define LINE_LAYER_MIDDLEEND

#include <Middleend/Layer_Statement.h>
#include <Frontend/Layer_Statement.h>

typedef struct Block_Context_IR Block_Context_IR;

struct Block_Context_IR {
    int b;
    int n;
    Var_IR var_defs[128];
    int var_def_cnt;

    StmtNode *next;
    Block_Context_IR *prev;
};

#endif