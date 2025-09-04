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
	STMT_TOKEN,
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
typedef struct FuncallArg FuncallArg;
typedef struct StmtNode StmtNode;
typedef struct Line_Context Line_Context;

struct Var {
	String  name;
	varMode mode;

	String type;
	Stmt  *defn_val;
	FuncallArg *arglist;
};

union StmtValue {
	Var       as_var;
	Token     as_token;
	Funcall  *as_funcall;
	StmtNode *as_block;
};

struct Stmt {
	StmtType  type;
	StmtValue value;
};

struct FuncallArg {
	FuncallArg *next;
	Stmt        value;
};

struct Funcall {
	String      name;
	FuncallArg *args;
};

struct StmtNode {
	Stmt statement;
	StmtNode *next;
};

Stmt stmt_fetch_next(Line_Context *ctx);

//-------------------------------- MIDDLE END --------------------------------
typedef struct Var_IR {
	String name;
	unsigned int mem_addr;
} Var_IR;

#endif