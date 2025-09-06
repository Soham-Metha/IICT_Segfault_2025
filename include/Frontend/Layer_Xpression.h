#ifndef EXPR_LAYER_FRONTEND
#define EXPR_LAYER_FRONTEND
#include <Utils/strings.h>
#include <Frontend/Layer_Line.h>

enum ExprType {
	EXPR_TYPE_STR,
	EXPR_TYPE_CHAR,
	EXPR_TYPE_NUMBER,
	EXPR_TYPE_NAME,
	EXPR_TYPE_OPEN_PAREN,
	EXPR_TYPE_CLOSING_PAREN,
	EXPR_TYPE_OPEN_CURLY,
	EXPR_TYPE_CLOSING_CURLY,
	EXPR_TYPE_STATEMENT_END,
	TOKEN_TYPE_THEN,
	TOKEN_TYPE_REPEAT,
	TOKEN_TYPE_COMMA,
	TOKEN_TYPE_COLON,
	TOKEN_TYPE_EQUAL,
	TOKEN_TYPE_EOL,
};

typedef struct Token Token;
typedef enum ExprType ExprType;
typedef struct Line_Context Line_Context;
struct Token {
	ExprType type;
	String text;
};

Token token_peek_next(Line_Context *ctx);
const char *token_get_name(ExprType type);
Token token_expect_next(Line_Context *ctx, ExprType expected);
bool token_consume(Line_Context *ctx);

#endif