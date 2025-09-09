#ifndef LINE_LAYER_MIDDLEEND
#define LINE_LAYER_MIDDLEEND

#include <Middleend/Layer_Statement.h>
#include <Frontend/Layer_Statement.h>

typedef struct Block_Context_IR Block_Context_IR;

typedef struct Var_IR {
	String name;
	int type;
	int mem_addr;
	bool has_def;
} Var_IR;

struct Block_Context_IR {
	int b;
	int n;
	Var_IR var_defs[128];
	int var_def_cnt;

	StmtNode *next;
	Block_Context_IR *prev;
};

// void IR_dump_expr(Expr tok, bool asVal);

// void IR_dump_statement(Block_Context_IR *ctx);

// void IR_dump_code_block(Block_Context_IR *ctx);

#endif