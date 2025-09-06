#ifndef EXPR_LAYER_FRONTEND
#define EXPR_LAYER_FRONTEND
#include <Utils/strings.h>
#include <Frontend/Layer_Line.h>
#include <Frontend/Layer_Tokens.h>
#include <stdint.h>

enum ExprType {
	EXPR_TYPE_STR,
	EXPR_TYPE_NUMBER,
	EXPR_TYPE_FUNCALL,
	EXPR_TYPE_VAR,
	EXPR_TYPE_BOOL,
	EXPR_TYPE_BIN_OPR,


	EXPR_TYPE_OPEN_CURLY,
	EXPR_TYPE_CLOSING_CURLY,
	EXPR_TYPE_STATEMENT_END,
	EXPR_TYPE_THEN,
	EXPR_TYPE_REPEAT,
	EXPR_TYPE_COLON,
	EXPR_TYPE_EQUAL,
	EXPR_TYPE_TOKEN,
};

typedef struct Funcall Funcall;
typedef struct FuncallArg FuncallArg;
typedef struct Expr Expr;
typedef enum ExprType ExprType;
typedef union ExprValue ExprValue;
typedef struct Line_Context Line_Context;

struct Funcall {
	String name;
	FuncallArg *args;
};

union ExprValue {
	String str;
	int64_t num;
	Funcall *funcall;
	bool boolean;
	String var_nm;
	Token token;
};

struct Expr {
	ExprType type;
	ExprValue as;
};

struct FuncallArg {
	Expr expr;
	FuncallArg *next;
};

Expr expr_peek_next(Line_Context *ctx);
const char *expr_get_name(ExprType type);
Expr expr_expect_next(Line_Context *ctx, ExprType expected);
bool expr_consume(Line_Context *ctx);
FuncallArg *functions_parse_arglist(Line_Context *ctx);

#endif