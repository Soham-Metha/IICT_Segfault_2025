#ifndef LINE_LAYER_MIDDLEEND
#define LINE_LAYER_MIDDLEEND

#include <Middleend/Layer_Statement.h>

typedef struct BlockScope {
	BlockScope *prev;
	Var_IR vars[128];
	int var_count;
	int fun_count;
} BlockScope;

typedef struct CodeBlockIR {
	BlockScope *scope;
} CodeBlockIR;

#endif