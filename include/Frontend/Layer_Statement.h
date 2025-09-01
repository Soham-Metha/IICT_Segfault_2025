#ifndef STMT_LAYER_FRONTEND
#define STMT_LAYER_FRONTEND
#include <Frontend/Layer_Line.h>
#include <Utils/strings.h>
#include <stdint.h>

enum StmtType {
	STMT_VARIABLE,
	STMT_LIT_INT,
	STMT_LIT_FLOAT,
	STMT_LIT_CHAR,
	STMT_LIT_STR,
	STMT_FUNCALL,
	STMT_FUNCALL_DEFINITION,
};

typedef struct Stmt Stmt;
typedef enum StmtType StmtType;
typedef union StmtValue StmtValue;
typedef struct Funcall Funcall;
typedef struct FuncallArg FuncallArg;

union StmtValue {
	String as_var;
	uint64_t as_int;
	double as_float;
	char as_char;
	String as_str;
	Funcall *as_funcall;
};

struct Stmt {
	StmtType type;
	StmtValue value;
};

struct FuncallArg {
	FuncallArg *next;
	Stmt value;
};

struct Funcall {
	String name;
	FuncallArg *args;
};

Stmt getNextStmt(String line);

#endif