#ifndef STMT_LAYER_FRONTEND
#define STMT_LAYER_FRONTEND
#include <Frontend/Layer_Xpression.h>
#include <Utils/strings.h>
#include <stdint.h>

enum StmtType {
	STMT_VAR,
	STMT_FUNCALL,
	STMT_BLOCK_START,
	STMT_BLOCK_END,
	STMT_MATCH,
	STMT_TOKEN,
	STMT_CONDITIONAL,
};

enum varMode {
	VAR_ACCS,
	VAR_DECL,
	VAR_DEFN,
	VAR_BOTH,
};

typedef struct Var Var;
typedef struct Stmt Stmt;
typedef enum StmtType StmtType;
typedef enum varMode varMode;
typedef union StmtValue StmtValue;
typedef struct Funcall Funcall;
typedef struct StmtNode FuncallArg;
typedef struct StmtNode StmtNode;
typedef struct Line_Context Line_Context;
typedef struct PatternMatch PatternMatch;
typedef struct StmtConditional StmtConditional;

struct Var {
	String  name;
	String  type;
	varMode mode;

	FuncallArg *arglist;
};

struct StmtConditional {
	bool  repeat;
	Token cond;
	CodeBlock body;
};

union StmtValue {
	Var             var;
	StmtConditional cond;
	Token           *token;
	Funcall         *funcall;
	StmtNode        *block;
	PatternMatch    *match;
};

struct Stmt {
	StmtType  type;
	StmtValue as;
};

struct Funcall {
	String name;
	FuncallArg *args;
};

struct StmtNode {
	Stmt statement;
	StmtNode *next;
};

struct PatternMatch {
	Stmt *cond;
	Stmt *body;
};

Stmt stmt_fetch_next(Line_Context *ctx);

#endif